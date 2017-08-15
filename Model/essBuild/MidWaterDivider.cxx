/*********************************************************************
  CombLayer : MCNP(X) Input builder

 * File:   essBuild/MidWaterDivider.cxx
 *
 * Copyright (c) 2004-2017 by Stuart Ansell
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
#include <array>
#include <algorithm>
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
#include "stringCombine.h"
#include "MatrixBase.h"
#include "Matrix.h"
#include "Vec3D.h"
#include "Quaternion.h"
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
#include "RuleSupport.h"
#include "Object.h"
#include "Qhull.h"
#include "Simulation.h"
#include "Vert2D.h"
#include "Convex2D.h"
#include "ModelSupport.h"
#include "MaterialSupport.h"
#include "generateSurf.h"
#include "LinkUnit.h"
#include "FixedComp.h"
#include "FixedOffset.h"
#include "ContainedComp.h"
#include "LayerComp.h"
#include "BaseMap.h"
#include "CellMap.h"
#include "AttachSupport.h"
#include "SurInter.h"
#include "geomSupport.h"
#include "ModBase.h"
#include "H2Wing.h"
#include "MidWaterDivider.h"

namespace essSystem
{

MidWaterDivider::MidWaterDivider(const std::string& baseKey,
				 const std::string& extraKey) :
  attachSystem::ContainedComp(),
  attachSystem::LayerComp(0,0),
  attachSystem::FixedComp(baseKey+extraKey,14),
  baseName(baseKey),
  divIndex(ModelSupport::objectRegister::Instance().cell(keyName)),
  cellIndex(divIndex+1)
  /*!
    Constructor BUT ALL variable are left unpopulated.
    \param baseKey :: Base Name for item in search
    \param extraKey :: extra [specialized] Name for item in search
  */
{}

MidWaterDivider::MidWaterDivider(const MidWaterDivider& A) :
  attachSystem::ContainedComp(A),attachSystem::LayerComp(A),
  attachSystem::FixedComp(A), baseName(A.baseName),
  divIndex(A.divIndex),cellIndex(A.cellIndex),cutLayer(A.cutLayer),
  midYStep(A.midYStep),
  midAngle(A.midAngle),length(A.length),height(A.height),
  wallThick(A.wallThick),modMat(A.modMat),wallMat(A.wallMat),
  modTemp(A.modTemp),
  edgeRadius(A.edgeRadius)
  /*!
    Copy constructor
    \param A :: MidWaterDivider to copy
  */
{}

MidWaterDivider&
MidWaterDivider::operator=(const MidWaterDivider& A)
  /*!
    Assignment operator
    \param A :: MidWaterDivider to copy
    \return *this
  */
{
  if (this!=&A)
    {
      attachSystem::ContainedComp::operator=(A);
      attachSystem::LayerComp::operator=(A);
      attachSystem::FixedComp::operator=(A);
      cellIndex=A.cellIndex;
      cutLayer=A.cutLayer;
      midYStep=A.midYStep;
      midAngle=A.midAngle;
      length=A.length;
      height=A.height;
      wallThick=A.wallThick;
      modMat=A.modMat;
      wallMat=A.wallMat;
      modTemp=A.modTemp;
      edgeRadius=A.edgeRadius;
    }
  return *this;
}

MidWaterDivider*
MidWaterDivider::clone() const
  /*!
    Virtual copy constructor
    \return new (this)
   */
{
  return new MidWaterDivider(*this);
}

MidWaterDivider::~MidWaterDivider()
  /*!
    Destructor
  */
{}

void
MidWaterDivider::populate(const FuncDataBase& Control)
  /*!
    Populate all the variables
    \param Control :: DataBase for variables
  */
{
  ELog::RegMethod RegA("MidWaterDivider","populate");

  cutLayer=Control.EvalDefVar<size_t>(keyName+"CutLayer",3);

  midYStep=Control.EvalVar<double>(keyName+"MidYStep");
  midAngle=Control.EvalVar<double>(keyName+"MidAngle");

  length=Control.EvalVar<double>(keyName+"Length");
  wallThick=Control.EvalVar<double>(keyName+"WallThick");
  topThick=Control.EvalDefVar<double>(keyName+"TopThick",0.0);
  baseThick=Control.EvalDefVar<double>(keyName+"BaseThick",0.0);

  modMat=ModelSupport::EvalMat<int>(Control,keyName+"ModMat");
  wallMat=ModelSupport::EvalMat<int>(Control,keyName+"WallMat");
  modTemp=Control.EvalVar<double>(keyName+"ModTemp");
  edgeRadius=Control.EvalVar<double>(keyName+"EdgeRadius");

  return;
}

void
MidWaterDivider::createUnitVector(const attachSystem::FixedComp& FC)
  /*!
    Create the unit vectors
    - Y Points down the MidWaterDivider direction
    - X Across the MidWaterDivider
    - Z up (towards the target)
    \param FC :: fixed Comp [and link comp]
  */
{
  ELog::RegMethod RegA("MidWaterDivider","createUnitVector");

  FixedComp::createUnitVector(FC);
  return;
}

void
MidWaterDivider::createLinks(const H2Wing& leftWing,
			     const H2Wing& rightWing)
  /*!
    Construct links:
    - 1+2 and 3+4 are the triangle surfaces (left/right)
    - 5+6 and 7+8 are corner points for view
    - 9+10 are top/bottom

    \param leftWing :: H2Wing connector  [left]
    \param rightWing :: H2Wing connector [right side]
  */
{
  ELog::RegMethod RegA("MidWaterDivider","createLinks");

  // main angles
  FixedComp::setLinkSurf(0, SMap.realSurf(divIndex+103));
  FixedComp::setLinkSurf(1, -SMap.realSurf(divIndex+104));
  FixedComp::setLinkSurf(2, SMap.realSurf(divIndex+123));
  FixedComp::setLinkSurf(3, -SMap.realSurf(divIndex+124));


  // small cutting edged
  FixedComp::setLinkSurf(5, SMap.realSurf(divIndex+111));
  FixedComp::setLinkSurf(6, SMap.realSurf(divIndex+112));
  FixedComp::setLinkSurf(7, SMap.realSurf(divIndex+131));
  FixedComp::setLinkSurf(8, SMap.realSurf(divIndex+132));

  std::vector<int> surfN;
  surfN.push_back(leftWing.getSignedLinkSurf(1));
  surfN.push_back(leftWing.getSignedLinkSurf(3));
  surfN.push_back(rightWing.getSignedLinkSurf(1));
  surfN.push_back(rightWing.getSignedLinkSurf(3));

  // Now deterermine point which are divider points
  const Geometry::Plane* midPlane=
    SMap.realPtr<Geometry::Plane>(divIndex+200);

  const std::vector<std::pair<int,int>> InterVec =
    {
      {103,104},{103,104},
      {123,124},{123,124},
      {111,-2},{112,-3},
      {131,-1},{132,-4}
    };
  const std::vector<Geometry::Vec3D> Axis
    ({Y,Y,-Y,-Y,Y,Y,-Y,-Y});


  for(size_t index=0;index<InterVec.size();index++)
    {
      const std::pair<int,int>& Item(InterVec[index]);
      const int SA(divIndex+Item.first);
      const int SB(Item.second>0 ? divIndex+Item.second :
		   surfN[static_cast<size_t>(-Item.second-1)]);
      const Geometry::Plane* PA=SMap.realPtr<Geometry::Plane>(SA);
      const Geometry::Plane* PB=SMap.realPtr<Geometry::Plane>(SB);
      FixedComp::setConnect
      	(index,SurInter::getPoint(PA,PB,midPlane),Axis[index]);
    }

  // full cut out
  std::string Out;
  HeadRule HR;

  Out=ModelSupport::getComposite(SMap,divIndex,"(-123 : 124) -131 -132 ");
  HR.procString(Out);
  HR.makeComplement();
  FixedComp::setLinkSurf(10,HR);
  FixedComp::setBridgeSurf(10,-SMap.realSurf(divIndex+100));

  // +ve Y
  Out=ModelSupport::getComposite(SMap,divIndex,"(-103 : 104)  -111 -112 ");
  HR.procString(Out);
  HR.makeComplement();
  FixedComp::setLinkSurf(11,HR);
  FixedComp::setBridgeSurf(11,SMap.realSurf(divIndex+100));


  FixedComp::setLinkSurf(12,SMap.realSurf(divIndex+100));
  FixedComp::setConnect(12,Origin,Y);

  return;
}

void
MidWaterDivider::createSurfaces()
  /*!
    Create All the surfaces
  */
{
  ELog::RegMethod RegA("MidWaterDivider","createSurface");

  // Mid divider
  ModelSupport::buildPlane(SMap,divIndex+100,Origin,Y);
  ModelSupport::buildPlane(SMap,divIndex+200,Origin,Z);
  ModelSupport::buildPlane(SMap,divIndex+300,Origin,X);

  // +Y section
  ModelSupport::buildPlaneRotAxis
    (SMap,divIndex+3,Origin+Y*midYStep,X,-Z,-midAngle/2.0);
  ModelSupport::buildPlaneRotAxis
    (SMap,divIndex+4,Origin+Y*midYStep,X,-Z,midAngle/2.0);

  // -Y section
  ModelSupport::buildPlaneRotAxis
    (SMap,divIndex+23,Origin-Y*midYStep,-X,-Z,-midAngle/2.0);
  ModelSupport::buildPlaneRotAxis
    (SMap,divIndex+24,Origin-Y*midYStep,-X,-Z,midAngle/2.0);

  // Make lengths:

  Geometry::Vec3D leftNorm(Y);
  Geometry::Quaternion::calcQRotDeg(-midAngle/2.0,Z).rotate(leftNorm);
  Geometry::Vec3D rightNorm(Y);
  Geometry::Quaternion::calcQRotDeg(midAngle/2.0,Z).rotate(rightNorm);

  ModelSupport::buildPlane(SMap,divIndex+11,Origin+leftNorm*length,leftNorm);
  ModelSupport::buildPlane(SMap,divIndex+12,Origin+rightNorm*length,rightNorm);

  // Length below [note reverse of normals]
  ModelSupport::buildPlane(SMap,divIndex+31,Origin-rightNorm*length,-rightNorm);
  ModelSupport::buildPlane(SMap,divIndex+32,Origin-leftNorm*length,-leftNorm);


  // Aluminum layers [+100]
  // +Y section
  const double LStep(midYStep+wallThick/sin(midAngle/2.0));
  ModelSupport::buildPlaneRotAxis
    (SMap,divIndex+103,Origin+Y*LStep,X,-Z,-midAngle/2.0);
  ModelSupport::buildPlaneRotAxis
    (SMap,divIndex+104,Origin+Y*LStep,X,-Z,midAngle/2.0);

  // -Y section

  ModelSupport::buildPlaneRotAxis
    (SMap,divIndex+123,Origin-Y*LStep,-X,-Z,-midAngle/2.0);
  ModelSupport::buildPlaneRotAxis
    (SMap,divIndex+124,Origin-Y*LStep,-X,-Z,midAngle/2.0);


  ModelSupport::buildPlane(SMap,divIndex+111,
			   Origin+leftNorm*(length+wallThick),leftNorm);
  ModelSupport::buildPlane(SMap,divIndex+112,
			   Origin+rightNorm*(length+wallThick),rightNorm);

  // Length below [note reverse of normals]
  ModelSupport::buildPlane(SMap,divIndex+131,
			   Origin-rightNorm*(wallThick+length),-rightNorm);
  ModelSupport::buildPlane(SMap,divIndex+132,
			   Origin-leftNorm*(wallThick+length),-leftNorm);


  if (topThick>Geometry::zeroTol)
    ModelSupport::buildPlane(SMap,divIndex+5,
                             Origin+Z*(height/2.0-topThick),Z);

  // Rounding of the edges
  const Geometry::Plane *pz = ModelSupport::buildPlane(SMap, divIndex+5, Origin, Z);

  int edgeOffset(divIndex+1000);
  std::array<Geometry::Vec3D,4> CPts; // water corners
  std::array<Geometry::Vec3D,4> APts; // points for Geometry::cornerCircle
  std::vector<int> side{11, 12, 32, 31};
  std::vector<int> front{4, 3,  24, 23};
  double thick=0.0;

  for (int j=0; j<2; j++) // water and Al
    {
      for (int i=0; i<4; i++) // four edges
	{
	  const int j100=static_cast<int>(j*100); // to avoid -Wsign-conversion complains
	  const size_t ii=static_cast<size_t>(i); // to avoid -Wsign-conversion complains
	  CPts[ii] = SurInter::getPoint(SMap.realPtr<Geometry::Plane>(divIndex+side[ii]+j100),
					SMap.realPtr<Geometry::Plane>(divIndex+front[ii] + j100), pz); // water corners
	  
	  if ((i==0) || (i==2))
	    APts[ii] = SurInter::getPoint(SMap.realPtr<Geometry::Plane>(divIndex+side[ii]+j100),
					  SMap.realPtr<Geometry::Plane>(divIndex+side[(i+3)%4]+j100), pz);
	  else if ((i==1) || (i==3))
	    APts[ii] = SurInter::getPoint(SMap.realPtr<Geometry::Plane>(divIndex+front[ii-1]+j100),
					  SMap.realPtr<Geometry::Plane>(divIndex+front[ii]+j100), pz);
	}
      
      for (size_t i=0; i<4; i++) // four edges
	{
	  const int ii(static_cast<int>(i)+1);

	  Geometry::Vec3D RCent;
	  RCent = Geometry::cornerCircleTouch(CPts[i], APts[i], APts[(i+1)%4], edgeRadius+thick);

	  std::pair<Geometry::Vec3D, Geometry::Vec3D> CutPair;
	  CutPair =    Geometry::cornerCircle(CPts[i], APts[i], APts[(i+1)%4], edgeRadius+thick);

	  // midNorm
	  Geometry::Vec3D a = (CPts[(i+1)%4]-CPts[i]).unit();
	  Geometry::Vec3D b = (CPts[(i+2)%4]-CPts[i]).unit();
	  Geometry::Vec3D MD = (a+b)/2.0;
	  
	  ModelSupport::buildPlane(SMap,edgeOffset+ii+20,
				   CutPair.first,CutPair.second,
				   CutPair.first+Z,MD);

	  ModelSupport::buildCylinder(SMap,edgeOffset+ii+6,
				      RCent,Z,edgeRadius+thick);
		  
	  
	}
      edgeOffset += 100;
      thick += wallThick;
    }
  
  return;
}

void
MidWaterDivider::createObjects(Simulation& System,
			       const H2Wing& leftWing,
			       const H2Wing& rightWing)
  /*!
    Adds the main components
    \param System :: Simulation to create objects in
    \param leftWing :: H2Wing connector  [left]
    \param rightWing :: H2Wing connector [right side]
  */
{
  ELog::RegMethod RegA("MidWaterDivider","createObjects");

  const std::string Base=leftWing.getLinkComplement(4);
  const std::string Top=leftWing.getLinkComplement(5);

  HeadRule LCut(leftWing.getLayerString(cutLayer,7));
  HeadRule RCut(rightWing.getLayerString(cutLayer,7));

  LCut.makeComplement();
  RCut.makeComplement();
  std::string Out;

  if (topThick>Geometry::zeroTol)
    {
      Out=ModelSupport::getComposite(SMap,divIndex,"100 (-3 : 4) -11 -12 -5 ");
      Out+=LCut.display()+RCut.display()+Base;
      System.addCell(MonteCarlo::Qhull(cellIndex++,modMat,modTemp,Out));

      Out=ModelSupport::getComposite(SMap,divIndex,"100 (-3 : 4) -11 -12 5 ");
      Out+=LCut.display()+RCut.display()+Top;
      System.addCell(MonteCarlo::Qhull(cellIndex++,wallMat,modTemp,Out));

      // Reverse side
      Out=ModelSupport::getComposite(SMap,divIndex,
                                     "-100 (-23 : 24) -31 -32 -5 ");
      Out+=LCut.display()+RCut.display()+Base;
      System.addCell(MonteCarlo::Qhull(cellIndex++,modMat,modTemp,Out));

      Out=ModelSupport::getComposite(SMap,divIndex,
                                     "-100 (-23 : 24) -31 -32 5 ");
      Out+=LCut.display()+RCut.display()+Top;
      System.addCell(MonteCarlo::Qhull(cellIndex++,wallMat,modTemp,Out));
    }
  else
    {
      Out=ModelSupport::getComposite(SMap,divIndex,"100 (-3 : 4) -11 -12 ");
      Out+=LCut.display()+RCut.display()+Base+Top;
      System.addCell(MonteCarlo::Qhull(cellIndex++,modMat,modTemp,Out));

      // Reverse layer
      Out=ModelSupport::getComposite(SMap,divIndex,"-100 (-23 : 24) -31 -32 ");
      Out+=LCut.display()+RCut.display()+Base+Top;
      System.addCell(MonteCarlo::Qhull(cellIndex++,modMat,modTemp,Out));

    }

  Out=ModelSupport::getComposite(SMap,divIndex,
				 "100 (-103 : 104) -111 -112 "
				 " ( (3  -4) : 11 : 12 ) ");

  Out+=LCut.display()+RCut.display()+Base+Top;
  System.addCell(MonteCarlo::Qhull(cellIndex++,wallMat,modTemp,Out));

  Out=ModelSupport::getComposite(SMap,divIndex,
				 "100 (-103 : 104)  -111 -112 ");
  addOuterSurf(Out);


  Out=ModelSupport::getComposite(SMap,divIndex,
				 "-100 (-123 : 124)  -131 -132 "
				 "((23  -24) : 31 : 32 )");
  Out+=LCut.display()+RCut.display()+Base+Top;
  System.addCell(MonteCarlo::Qhull(cellIndex++,wallMat,modTemp,Out));
  Out=ModelSupport::getComposite(SMap,divIndex,
				 "-100 (-123 : 124) -131 -132 ");
  addOuterUnionSurf(Out);


  return;
}

void
MidWaterDivider::cutOuterWing(Simulation& System,
			      const H2Wing& leftWing,
			      const H2Wing& rightWing) const
  /*!
    Cut the outer surface layer of the wings with the
    exclude version of the water layer
    \param System :: Simuation to extract objects rom
    \param leftWing :: Left wing object
    \param rightWing :: Right wing object
  */
{
  ELog::RegMethod RegA("MidWaterDivider","cutOuterWing");

  const size_t lWing=leftWing.getNLayers();
  const size_t rWing=rightWing.getNLayers();

  const std::string LBase=
    leftWing.getLinkComplement(4)+leftWing.getLinkComplement(5);
  const std::string RBase=
    rightWing.getLinkComplement(4)+rightWing.getLinkComplement(5);

  HeadRule cutRule;
  std::string Out;
  if (cutLayer+1<lWing)
    {
      const int cellA=leftWing.getCell("Outer");

      MonteCarlo::Qhull* OPtr=System.findQhull(cellA);
      if (!OPtr)
	throw ColErr::InContainerError<int>(cellA,"leftWing Cell: Outer");
      Out=ModelSupport::getComposite(SMap,divIndex,
				     " (100: -11) (-100:-31) ");
      cutRule.procString(Out);
      cutRule.makeComplement();
      OPtr->addSurfString(cutRule.display());
    }
  if (cutLayer+1<rWing)
    {
      const int cellB=rightWing.getCell("Outer");
      MonteCarlo::Qhull* OPtr=System.findQhull(cellB);
      if (!OPtr)
	throw ColErr::InContainerError<int>(cellB,"rightWing Cell: Outer");
      Out=ModelSupport::getComposite(SMap,divIndex,
				     " (100:-12) : (-100:-32) ");
      cutRule.procString(Out);
      cutRule.makeComplement();
      OPtr->addSurfString(cutRule.display());
    }
  return;
}


Geometry::Vec3D
MidWaterDivider::getSurfacePoint(const size_t,
			const long int) const
  /*!
    Given a side and a layer calculate the link point
    \param :: layer, 0 is inner moderator [0-6]
    \param :: Side [0-3] // mid sides
    \return Surface point
  */
{
  ELog::RegMethod RegA("MidWaterDivider","getSurfacePoint");
  throw ColErr::AbsObjMethod("Not implemented yet");
}

int
MidWaterDivider::getLayerSurf(const size_t,
                              const long int) const
  /*!
    Given a side and a layer calculate the link point
    \param :: layer, 0 is inner moderator [0-3]
    \param :: Side [0-3] // mid sides
    \return Surface point
  */
{
  ELog::RegMethod RegA("MidWaterDivider","getLayerSurf");
  throw ColErr::AbsObjMethod("Not implemented yet");
}

std::string
MidWaterDivider::getLayerString(const size_t,
                                const long int) const
  /*!
    Given a side and a layer calculate the link point
    \param :: layer, 0 is inner moderator [0-6]
    \param :: Side [0-3] // mid sides
    \return Surface point
  */
{
  ELog::RegMethod RegA("MidWaterDivider","getLayerString");
  throw ColErr::AbsObjMethod("Not implemented yet");
}


void
MidWaterDivider::createAll(Simulation& System,
			   const attachSystem::FixedComp& FC,
			   const H2Wing& LA,
			   const H2Wing& RA)
  /*!
    Generic function to create everything
    \param System :: Simulation item
    \param FC :: Fixed object just for origin/axis
    \param LA :: Left node object
    \param RA :: Right node object
  */
{
  ELog::RegMethod RegA("MidWaterDivider","createAll");

  populate(System.getDataBase());
  height=LA.getLinkDistance(5,6)-topThick;

  createUnitVector(FC);
  createSurfaces();
  createObjects(System,LA,RA);
  cutOuterWing(System,LA,RA);
  createLinks(LA,RA);
  insertObjects(System);
  return;
}

}  // NAMESPACE essSystem
