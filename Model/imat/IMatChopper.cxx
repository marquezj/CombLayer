/********************************************************************* 
  CombLayer : MCNP(X) Input builder
 
 * File:   imat/IMatChopper.cxx
 *
 * Copyright (c) 2004-2018 by Stuart Ansell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 *
 ****************************************************************************/
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cmath>
#include <complex>
#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <numeric>
#include <memory>

#include "Exception.h"
#include "FileReport.h"
#include "GTKreport.h"
#include "NameStack.h"
#include "RegMethod.h"
#include "OutputLog.h"
#include "BaseVisit.h"
#include "BaseModVisit.h"
#include "support.h"
#include "MatrixBase.h"
#include "Matrix.h"
#include "Vec3D.h"
#include "Quaternion.h"
#include "localRotate.h"
#include "masterRotate.h"
#include "Surface.h"
#include "surfIndex.h"
#include "surfRegister.h"
#include "objectRegister.h"
#include "surfEqual.h"
#include "surfDivide.h"
#include "surfDIter.h"
#include "Quadratic.h"
#include "Plane.h"
#include "Cylinder.h"
#include "Line.h"
#include "Rules.h"
#include "varList.h"
#include "Code.h"
#include "FuncDataBase.h"
#include "HeadRule.h"
#include "Object.h"
#include "groupRange.h"
#include "objectGroups.h"
#include "Simulation.h"
#include "ModelSupport.h"
#include "generateSurf.h"
#include "LinkUnit.h"
#include "FixedComp.h"
#include "FixedGroup.h"
#include "ContainedComp.h"
#include "SpaceCut.h"
#include "ContainedGroup.h"
#include "IMatChopper.h"

namespace imatSystem
{

IMatChopper::IMatChopper(const std::string& Key)  :
  attachSystem::ContainedComp(),
  attachSystem::FixedGroup(Key,"Main",6,"Beam",2),
  innerVoid(0)
  /*!
    Constructor BUT ALL variable are left unpopulated.
    \param Key :: Name for item in search
  */
{}


IMatChopper::~IMatChopper() 
 /*!
   Destructor
 */
{}

void
IMatChopper::populate(const Simulation& System)
 /*!
   Populate all the variables
   \param System :: Simulation to use
 */
{
  ELog::RegMethod RegA("IMatChopper","populate");
  
  const FuncDataBase& Control=System.getDataBase();

  // First get inner widths:
  xStep=Control.EvalVar<double>(keyName+"XStep");
  yStep=Control.EvalVar<double>(keyName+"YStep");
  zStep=Control.EvalVar<double>(keyName+"ZStep");

  xyAngle=Control.EvalVar<double>(keyName+"XYAngle");
  zAngle=Control.EvalVar<double>(keyName+"ZAngle");

  // void
  length=Control.EvalVar<double>(keyName+"Length");
  height=Control.EvalVar<double>(keyName+"Height");
  depth=Control.EvalVar<double>(keyName+"Depth");
  left=Control.EvalVar<double>(keyName+"Left");
  right=Control.EvalVar<double>(keyName+"Right");

  // FE
  feWidth=Control.EvalVar<double>(keyName+"FeWidth");
  feFront=Control.EvalVar<double>(keyName+"FeFront");
  feBack=Control.EvalVar<double>(keyName+"FeBack");
  feBase=Control.EvalVar<double>(keyName+"FeBase");
  feTop=Control.EvalVar<double>(keyName+"FeTop");

  // WAll
  wallWidth=Control.EvalVar<double>(keyName+"WallWidth");
  wallFront=Control.EvalVar<double>(keyName+"WallFront");
  wallBack=Control.EvalVar<double>(keyName+"WallBack");
  wallBase=Control.EvalVar<double>(keyName+"WallBase");
  wallTop=Control.EvalVar<double>(keyName+"WallTop");
 
  feMat=Control.EvalVar<int>(keyName+"FeMat");
  wallMat=Control.EvalVar<int>(keyName+"WallMat");

  return;
}
  
void
IMatChopper::createUnitVector(const attachSystem::FixedGroup& TC)
  /*!
    Create the unit vectors
    - Y Points towards the beamline
    - X Across the Face
    - Z up (towards the target)
    \param TC :: A second track to use the beam direction as basis
  */
{
  ELog::RegMethod RegA("IMatChopper","createUnitVector");

  attachSystem::FixedComp& mainFC=FixedGroup::getKey("Main");
  attachSystem::FixedComp& beamFC=FixedGroup::getKey("Beam");

  mainFC.createUnitVector(TC.getKey("Main"));
  beamFC.createUnitVector(TC.getKey("Beam"));

  mainFC.setCentre(beamFC.getCentre());
  mainFC.applyShift(xStep,yStep,zStep);
  beamFC.applyShift(xStep,yStep,zStep);
  beamFC.applyAngleRotate(xyAngle,zAngle);
  
  return;
}

void
IMatChopper::createSurfaces()
  /*!
    Create All the surfaces
    -- Outer plane is going 
  */
{
  ELog::RegMethod RegA("IMatChopper","createSurfaces");

  const attachSystem::FixedComp& beamFC=FixedGroup::getKey("Beam");

  const Geometry::Vec3D& bX(beamFC.getX());
  const Geometry::Vec3D& bY(beamFC.getY());
  const Geometry::Vec3D& bZ(beamFC.getZ());
  
  ModelSupport::buildPlane(SMap,buildIndex+1,Origin,bY);
  ModelSupport::buildPlane(SMap,buildIndex+2,Origin+bY*length,bY);
  ModelSupport::buildPlane(SMap,buildIndex+3,Origin-bX*left,bX);
  ModelSupport::buildPlane(SMap,buildIndex+4,Origin+bX*right,bX);
  ModelSupport::buildPlane(SMap,buildIndex+5,Origin-bZ*depth,bZ);
  ModelSupport::buildPlane(SMap,buildIndex+6,Origin+bZ*height,bZ);



  ModelSupport::buildPlane(SMap,buildIndex+11,Origin-bY*feBack,bY);
  ModelSupport::buildPlane(SMap,buildIndex+12,Origin+bY*(length+feFront),bY);
  ModelSupport::buildPlane(SMap,buildIndex+13,Origin-bX*(left+feWidth),bX);
  ModelSupport::buildPlane(SMap,buildIndex+14,Origin+bX*(right+feWidth),bX);
  ModelSupport::buildPlane(SMap,buildIndex+15,Origin-bZ*(depth+feBase),bZ);
  ModelSupport::buildPlane(SMap,buildIndex+16,Origin+bZ*(height+feTop),bZ);



  ModelSupport::buildPlane(SMap,buildIndex+21,
			   Origin-bY*(feBack+wallBack),bY);
  ModelSupport::buildPlane(SMap,buildIndex+22,
			   Origin+bY*(length+feFront+wallFront),bY);
  ModelSupport::buildPlane(SMap,buildIndex+23,
			   Origin-bX*(left+feWidth+wallWidth),bX);
  ModelSupport::buildPlane(SMap,buildIndex+24,
			   Origin+bX*(right+feWidth+wallWidth),bX);
  ModelSupport::buildPlane(SMap,buildIndex+25,
			   Origin-bZ*(depth+feBase+wallBase),bZ);
  ModelSupport::buildPlane(SMap,buildIndex+26,
			   Origin+bZ*(height+feTop+wallTop),bZ);

  return;
}

void
IMatChopper::createObjects(Simulation& System)
  /*!
    Adds the BeamLne components
    \param System :: Simulation to add beamline to
  */
{
  ELog::RegMethod RegA("IMatChopper","createObjects");
  
  std::string Out;
  Out=ModelSupport::getComposite(SMap,buildIndex," 21 -22 23 -24 25 -26 ");
  addOuterSurf(Out);

  // Inner void cell:
  Out=ModelSupport::getComposite(SMap,buildIndex,"1 -2 3 -4 5 -6 ");
  System.addCell(MonteCarlo::Object(cellIndex++,0,0.0,Out));

  // Fe layer:
  Out=ModelSupport::getComposite(SMap,buildIndex,"11 -12 13 -14 15 -16 "
				 "(-1:2:-3:4:-5:6) ");
  System.addCell(MonteCarlo::Object(cellIndex++,feMat,0.0,Out));

  // Wall layer:
  Out=ModelSupport::getComposite(SMap,buildIndex,"21 -22 23 -24 25 -26 "
				 " (-11:12:-13:14:-15:16) ");
  System.addCell(MonteCarlo::Object(cellIndex++,wallMat,0.0,Out));
  
  return;
}

void
IMatChopper::createLinks()
  /*!
    Create All the links:
    - 0 : First surface
    - 1 : Exit surface
  */
{
  ELog::RegMethod RegA("IMatChopper","createLinks");

  attachSystem::FixedComp& mainFC=FixedGroup::getKey("Main");
  attachSystem::FixedComp& beamFC=FixedGroup::getKey("Beam");

  const Geometry::Vec3D bY(beamFC.getY());

  mainFC.setConnect(1,Origin+bY*length,bY);
  beamFC.setConnect(1,Origin+bY*length,bY);

  mainFC.setConnect(0,Origin,-bY);      // Note always to the moderator
  FixedComp::setLinkSurf(1,SMap.realSurf(buildIndex+2));

  return;
}

void
IMatChopper::createAll(Simulation& System,
		       const attachSystem::FixedGroup& TC)
  /*!
    Global creation of the vac-vessel
    \param System :: Simulation to add vessel to
    \param TC :: IMatGuide
  */
{
  ELog::RegMethod RegA("IMatChopper","createAll");
  populate(System);

  createUnitVector(TC);
  createSurfaces();
  createObjects(System);
  insertObjects(System); 
  createLinks();

  return;
}

  
}  // NAMESPACE imatSystem
