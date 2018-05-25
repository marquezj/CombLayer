/********************************************************************* 
  CombLayer : MCNP(X) Input builder
 
 * File:   cosax/BremColl.cxx
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
#include <memory>
#include <array>

#include "Exception.h"
#include "FileReport.h"
#include "GTKreport.h"
#include "NameStack.h"
#include "RegMethod.h"
#include "OutputLog.h"
#include "BaseVisit.h"
#include "BaseModVisit.h"
#include "support.h"
#include "stringCombine.h"
#include "MatrixBase.h"
#include "Matrix.h"
#include "Vec3D.h"
#include "Surface.h"
#include "surfIndex.h"
#include "surfRegister.h"
#include "objectRegister.h"
#include "Quadratic.h"
#include "Plane.h"
#include "Cylinder.h"
#include "Rules.h"
#include "varList.h"
#include "Code.h"
#include "FuncDataBase.h"
#include "HeadRule.h"
#include "Object.h"
#include "Qhull.h"
#include "Simulation.h"
#include "ModelSupport.h"
#include "MaterialSupport.h"
#include "generateSurf.h"
#include "LinkUnit.h"  
#include "FixedComp.h"
#include "FixedOffset.h"
#include "ContainedComp.h"
#include "ContainedSpace.h"
#include "BaseMap.h"
#include "CellMap.h"
#include "FrontBackCut.h"

#include "BremColl.h"

namespace xraySystem
{

BremColl::BremColl(const std::string& Key) :
  attachSystem::FixedOffset(Key,2),
  attachSystem::ContainedSpace(),attachSystem::CellMap(),
  attachSystem::FrontBackCut()
  /*!
    Constructor BUT ALL variable are left unpopulated.
    \param Key :: KeyName
  */
{}
  
BremColl::~BremColl() 
  /*!
    Destructor
  */
{}

void
BremColl::populate(const FuncDataBase& Control)
  /*!
    Populate all the variables
    \param Control :: DataBase of variables
  */
{
  ELog::RegMethod RegA("BremColl","populate");
  
  FixedOffset::populate(Control);

  // Void + Fe special:
  height=Control.EvalVar<double>(keyName+"Height");
  width=Control.EvalVar<double>(keyName+"Width");
  length=Control.EvalVar<double>(keyName+"Length");
  wallThick=Control.EvalVar<double>(keyName+"WallThick");

  flangeARadius=Control.EvalPair<double>(keyName+"FlangeARadius",
					 keyName+"FlangeRadius");

  flangeALength=Control.EvalPair<double>(keyName+"FlangeALength",
					 keyName+"FlangeLength");
  flangeBRadius=Control.EvalPair<double>(keyName+"FlangeBRadius",
					 keyName+"FlangeRadius");

  flangeBLength=Control.EvalPair<double>(keyName+"FlangeBLength",
					 keyName+"FlangeLength");

  innerRadius=Control.EvalDefVar<double>
    (keyName+"InnerRadius",std::max(height,width)/2.0);


  holeXStep=Control.EvalVar<double>(keyName+"HoleXStep");
  holeZStep=Control.EvalVar<double>(keyName+"HoleZStep");
  holeAHeight=Control.EvalVar<double>(keyName+"HoleAHeight");
  holeAWidth=Control.EvalVar<double>(keyName+"HoleAWidth");
  holeBHeight=Control.EvalVar<double>(keyName+"HoleBHeight");
  holeBWidth=Control.EvalVar<double>(keyName+"HoleBWidth");
  
  voidMat=ModelSupport::EvalDefMat<int>(Control,keyName+"VoidMat",0);
  innerMat=ModelSupport::EvalMat<int>(Control,keyName+"InnerMat");
  wallMat=ModelSupport::EvalMat<int>(Control,keyName+"WallMat");

  return;
}

void
BremColl::createUnitVector(const attachSystem::FixedComp& FC,
			   const long int sideIndex)
  /*!
    Create the unit vectors
    \param FC :: Fixed component to link to
    \param sideIndex :: Link point and direction [0 for origin]
  */
{
  ELog::RegMethod RegA("BremColl","createUnitVector");

  FixedComp::createUnitVector(FC,sideIndex);
  Origin+=Y*(flangeALength+length/2.0);
  applyOffset();

  return;
}


void
BremColl::createSurfaces()
  /*!
    Create the surfaces
  */
{
  ELog::RegMethod RegA("BremColl","createSurfaces");

  // Do outer surfaces (vacuum ports)
  if (!frontActive())
    {
      ModelSupport::buildPlane(SMap,buildIndex+1,
			       Origin-Y*(flangeALength+length/2.0),Y);
      setFront(SMap.realSurf(buildIndex+1));
    }
  if (!backActive())
    {
      ModelSupport::buildPlane(SMap,buildIndex+2,
			       Origin+Y*(flangeBLength+length/2.0),Y);
      setBack(-SMap.realSurf(buildIndex+2));
    }

  // hole [front]:
  const Geometry::Vec3D holeFront=
    Origin+X*holeXStep+Z*holeZStep-Y*(length/2.0);
  const Geometry::Vec3D holeBack=
    Origin+X*holeXStep+Z*holeZStep+Y*(length/2.0);

  ModelSupport::buildPlane(SMap,buildIndex+1003,
			   holeFront-X*(holeAWidth/2.0),
			   holeBack-X*(holeBWidth/2.0),
			   holeBack-X*(holeBWidth/2.0)+Z,
			   X);
  ModelSupport::buildPlane(SMap,buildIndex+1004,
			   holeFront+X*(holeAWidth/2.0),
			   holeBack+X*(holeBWidth/2.0),
			   holeBack+X*(holeBWidth/2.0)+Z,
			   X);
  ModelSupport::buildPlane(SMap,buildIndex+1005,
			   holeFront-Z*(holeAHeight/2.0),
			   holeBack-Z*(holeBHeight/2.0),
			   holeBack-Z*(holeBHeight/2.0)+X,
			   Z);
  ModelSupport::buildPlane(SMap,buildIndex+1006,
			   holeFront+Z*(holeAHeight/2.0),
			   holeBack+Z*(holeBHeight/2.0),
			   holeBack+Z*(holeBHeight/2.0)+X,
			   Z);
  
  // void space:
  ModelSupport::buildPlane(SMap,buildIndex+3,Origin-X*(width/2.0),X);
  ModelSupport::buildPlane(SMap,buildIndex+4,Origin+X*(width/2.0),X);
  ModelSupport::buildPlane(SMap,buildIndex+5,Origin-Z*(height/2.0),Z);
  ModelSupport::buildPlane(SMap,buildIndex+6,Origin+Z*(height/2.0),Z);

  ModelSupport::buildPlane(SMap,buildIndex+13,
			      Origin-X*(wallThick+width/2.0),X);
  ModelSupport::buildPlane(SMap,buildIndex+14,
			      Origin+X*(wallThick+width/2.0),X);
  ModelSupport::buildPlane(SMap,buildIndex+15,
			      Origin-Z*(wallThick+height/2.0),Z);
  ModelSupport::buildPlane(SMap,buildIndex+16,
			      Origin+Z*(wallThick+height/2.0),Z);

  ModelSupport::buildPlane(SMap,buildIndex+101,
			   Origin-Y*(length/2.0),Y);
  ModelSupport::buildPlane(SMap,buildIndex+102,
			   Origin+Y*(length/2.),Y);

  ModelSupport::buildCylinder(SMap,buildIndex+7,Origin,Y,innerRadius);
  ModelSupport::buildCylinder(SMap,buildIndex+17,Origin,Y,flangeARadius);
  ModelSupport::buildCylinder(SMap,buildIndex+27,Origin,Y,flangeBRadius);

  return;
}

void
BremColl::createObjects(Simulation& System)
  /*!
    Adds the vacuum box
    \param System :: Simulation to create objects in
   */
{
  ELog::RegMethod RegA("BremColl","createObjects");

  const std::string frontSurf(frontRule());
  const std::string backSurf(backRule());

  std::string Out;
  
  Out=ModelSupport::getComposite(SMap,buildIndex,
				 "101 -102 1003 -1004 1005 -1006 ");
  makeCell("Void",System,cellIndex++,voidMat,0.0,Out);
  
  Out=ModelSupport::getComposite
    (SMap,buildIndex," 101 -102 3 -4 5 -6 (-1003: 1004 : -1005: 1006)");
  makeCell("Inner",System,cellIndex++,innerMat,0.0,Out);

  Out=ModelSupport::getComposite
    (SMap,buildIndex,"101 -102 13 -14 15 -16 (-3:4:-5:6)");
  makeCell("Wall",System,cellIndex++,wallMat,0.0,Out);

  // flanges
  Out=ModelSupport::getComposite(SMap,buildIndex," -7 -101 ");
  makeCell("FrontVoid",System,cellIndex++,voidMat,0.0,Out+frontSurf);
  Out=ModelSupport::getComposite(SMap,buildIndex," -7  102 ");
  makeCell("BackVoid",System,cellIndex++,voidMat,0.0,Out+backSurf);

  Out=ModelSupport::getComposite(SMap,buildIndex," -17 7 -101 ");
  makeCell("FrontFlange",System,cellIndex++,wallMat,0.0,Out+frontSurf);
  Out=ModelSupport::getComposite(SMap,buildIndex," -27 7 102 ");
  makeCell("FrontFlange",System,cellIndex++,wallMat,0.0,Out+backSurf);

  
  Out=ModelSupport::getComposite(SMap,buildIndex," 101 -102 13 -14 15 -16");
  addOuterSurf(Out);
  Out=ModelSupport::getComposite(SMap,buildIndex," -17 -101 ");
  addOuterUnionSurf(Out+frontSurf);
  Out=ModelSupport::getComposite(SMap,buildIndex," -27 102 ");
  addOuterUnionSurf(Out+backSurf);

  return;
}

void
BremColl::createLinks()
  /*!
    Determines the link point on the outgoing plane.
    It must follow the beamline, but exit at the plane.
    Port position are used for first two link points
    Note that 0/1 are the flange surfaces
  */
{
  ELog::RegMethod RegA("BremColl","createLinks");

  // port centre
  
  FrontBackCut::createFrontLinks(*this,Origin,Y); 
  FrontBackCut::createBackLinks(*this,Origin,Y);  
  
  return;
}

  
  
  
void
BremColl::createAll(Simulation& System,
		     const attachSystem::FixedComp& FC,
		     const long int FIndex)
  /*!
    Generic function to create everything
    \param System :: Simulation item
    \param FC :: FixedComp
    \param FIndex :: Fixed Index
  */
{
  ELog::RegMethod RegA("BremColl","createAll(FC)");

  populate(System.getDataBase());
  createUnitVector(FC,FIndex);
  createSurfaces();    
  createObjects(System);
  
  createLinks();
  
  insertObjects(System);   


  return;
}
  
}  // NAMESPACE xraySystem
