/********************************************************************* 
  CombLayer : MCNP(X) Input builder
 
 * File:   balder/FrontEndCave.cxx
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
#include "Quaternion.h"
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
#include "FixedGroup.h"
#include "FixedOffset.h"
#include "ContainedComp.h"
#include "SpaceCut.h"
#include "ContainedSpace.h"
#include "ExternalCut.h"
#include "BaseMap.h"
#include "CellMap.h"

#include "FrontEndCave.h"

namespace xraySystem
{

FrontEndCave::FrontEndCave(const std::string& Key) : 
  attachSystem::FixedOffset(Key,10),
  attachSystem::ContainedSpace(),
  attachSystem::ExternalCut(),
  attachSystem::CellMap()
  /*!
    Constructor BUT ALL variable are left unpopulated.
    \param Key :: KeyName
  */
{
  nameSideIndex(2,"outerWall");
  nameSideIndex(3,"ringWall");
  nameSideIndex(7,"ringAngleWall");
  nameSideIndex(9,"connectPt");
}


FrontEndCave::~FrontEndCave() 
  /*!
    Destructor
  */
{}

void
FrontEndCave::populate(const FuncDataBase& Control)
  /*!
    Populate all the variables
    \param Control :: DataBase of variables
  */
{
  ELog::RegMethod RegA("FrontEndCave","populate");
  
  FixedOffset::populate(Control);

  frontWallThick=Control.EvalVar<double>(keyName+"FrontWallThick");
  
  // Void + Fe special:
  length=Control.EvalVar<double>(keyName+"Length");
  ringGap=Control.EvalVar<double>(keyName+"RingGap");
  outerGap=Control.EvalVar<double>(keyName+"OuterGap");

  outerWallThick=Control.EvalVar<double>(keyName+"OuterWallThick");
  ringWallThick=Control.EvalVar<double>(keyName+"RingWallThick");
    
  floorDepth=Control.EvalVar<double>(keyName+"FloorDepth");
  floorThick=Control.EvalVar<double>(keyName+"FloorThick");
  roofHeight=Control.EvalVar<double>(keyName+"RoofHeight");
  roofThick=Control.EvalVar<double>(keyName+"RoofThick");

  segmentLength=Control.EvalVar<double>(keyName+"SegmentLength");
  segmentAngle=Control.EvalVar<double>(keyName+"SegmentAngle");


  frontHoleRadius=Control.EvalVar<double>(keyName+"FrontHoleRadius");

  frontWallMat=ModelSupport::EvalMat<int>(Control,keyName+"FrontWallMat");
  wallMat=ModelSupport::EvalMat<int>(Control,keyName+"WallMat");
  floorMat=ModelSupport::EvalMat<int>(Control,keyName+"FloorMat");
  roofMat=ModelSupport::EvalMat<int>(Control,keyName+"RoofMat");

  return;
}

void
FrontEndCave::createUnitVector(const attachSystem::FixedComp& FC,
			       const long int sideIndex)
  /*!
    Create the unit vectors
    \param FC :: Fixed component to link to
    \param sideIndex :: Link point and direction [0 for origin]
  */
{
  ELog::RegMethod RegA("FrontEndCave","createUnitVector");

  FixedComp::createUnitVector(FC,sideIndex);
  applyOffset();
  return;
}
 
void
FrontEndCave::createSurfaces()
  /*!
    Create the surfaces
  */
{
  ELog::RegMethod RegA("FrontEndCave","createSurfaces");

  // Inner void
  if (!ExternalCut::isActive("front"))
    {
      ModelSupport::buildPlane(SMap,buildIndex+1,Origin,Y);
      ExternalCut::setCutSurf("front",SMap.realSurf(buildIndex+1));
    }
  ModelSupport::buildPlane(SMap,buildIndex+2,Origin+Y*length,Y);
  ModelSupport::buildPlane(SMap,buildIndex+3,Origin-X*outerGap,X);
  ModelSupport::buildPlane(SMap,buildIndex+4,Origin+X*ringGap,X);
  ModelSupport::buildPlane(SMap,buildIndex+5,Origin-Z*floorDepth,Z);
  ModelSupport::buildPlane(SMap,buildIndex+6,Origin+Z*roofHeight,Z);


  ModelSupport::buildPlane
    (SMap,buildIndex+12,Origin+Y*(frontWallThick+length),Y);
  ModelSupport::buildPlane
     (SMap,buildIndex+13,Origin-X*(outerWallThick+outerGap),X);
  ModelSupport::buildPlane
     (SMap,buildIndex+14,Origin+X*(ringWallThick+ringGap),X);
  ModelSupport::buildPlane
     (SMap,buildIndex+15,Origin-Z*(floorThick+floorDepth),Z);
  ModelSupport::buildPlane
     (SMap,buildIndex+16,Origin+Z*(roofThick+roofHeight),Z);


  // Angle divider

  const double L1=length+frontWallThick-segmentLength;
  Geometry::Vec3D newBeamPt=Origin+
    Y*(length+frontWallThick)+
    X*(-outerGap/cos(M_PI*segmentAngle/180)+L1*sin(M_PI*segmentAngle/180.0));
  Geometry::Vec3D RPoint(Origin+X*ringGap+Y*segmentLength);

  
  ModelSupport::buildPlane(SMap,buildIndex+102,RPoint,Y);
  // inner surface

  ModelSupport::buildPlaneRotAxis
    (SMap,buildIndex+103,newBeamPt,X,-Z,segmentAngle);

  ModelSupport::buildPlaneRotAxis
    (SMap,buildIndex+104,RPoint,X,-Z,segmentAngle);

  RPoint += X*ringWallThick;
  ModelSupport::buildPlaneRotAxis
    (SMap,buildIndex+114,RPoint,X,-Z,segmentAngle);

  // exit hole
  ModelSupport::buildCylinder(SMap,buildIndex+107,Origin,Y,frontHoleRadius);
  
  return;
}

void
FrontEndCave::createObjects(Simulation& System)
  /*!
    Adds the main objects
    \param System :: Simulation to create objects in
   */
{
  ELog::RegMethod RegA("FrontEndCave","createObjects");

  const std::string fStr=getRuleStr("front");
  std::string Out;

  Out=ModelSupport::getComposite(SMap,buildIndex," -2 3 (-4:-104) 5 -6 ");
  makeCell("Void",System,cellIndex++,0,0.0,Out+fStr);
  
  Out=ModelSupport::getComposite(SMap,buildIndex," 2 -12 13 -103 5 -6 107 ");
  makeCell("FrontWall",System,cellIndex++,wallMat,0.0,Out);

  Out=ModelSupport::getComposite(SMap,buildIndex," 2 -12 103 -104 5 -6 ");
  makeCell("FrontWallVoid",System,cellIndex++,0,0.0,Out);

  Out=ModelSupport::getComposite(SMap,buildIndex," 2 -12 104 -114 5 -6 ");
  makeCell("FrontWallRing",System,cellIndex++,wallMat,0.0,Out);

  Out=ModelSupport::getComposite(SMap,buildIndex," 2 -12 -107 ");
  makeCell("FrontWallHole",System,cellIndex++,0,0.0,Out);

  Out=ModelSupport::getComposite(SMap,buildIndex," -2 -3 13 5 -6 ");
  makeCell("OuterWall",System,cellIndex++,wallMat,0.0,Out+fStr);

  Out=ModelSupport::getComposite(SMap,buildIndex," -102 4 -14 5 -6 ");
  makeCell("RingAWall",System,cellIndex++,wallMat,0.0,Out+fStr);

  Out=ModelSupport::getComposite(SMap,buildIndex," 102 -2 104 -114 5 -6 ");
  makeCell("RingBWall",System,cellIndex++,wallMat,0.0,Out);

  Out=ModelSupport::getComposite(SMap,buildIndex," -102 13 -14 -5 15 ");
  makeCell("FloorA",System,cellIndex++,floorMat,0.0,Out+fStr);

  Out=ModelSupport::getComposite(SMap,buildIndex," 102 -12 13 -114 -5 15 ");
  makeCell("FloorB",System,cellIndex++,floorMat,0.0,Out);

  Out=ModelSupport::getComposite(SMap,buildIndex," -102 13 -14 6 -16 ");
  makeCell("RoofA",System,cellIndex++,floorMat,0.0,Out+fStr);

  Out=ModelSupport::getComposite(SMap,buildIndex," 102 -12 13 -114 6 -16 ");
  makeCell("RoofB",System,cellIndex++,floorMat,0.0,Out);
  
  Out=ModelSupport::getComposite(SMap,buildIndex," -12 13 (-14:-114) 15 -16 ");
  addOuterSurf(Out+fStr);      

  return;
}

void
FrontEndCave::createLinks()
  /*!
    Determines the link point on the outgoing plane.
    It must follow the beamline, but exit at the plane
  */
{
  ELog::RegMethod RegA("FrontEndCave","createLinks");
  
  ExternalCut::createLink("front",*this,0,Origin,Y);

  setConnect(1,Origin+Y*(frontWallThick+length),Y);
  setLinkSurf(1,SMap.realSurf(buildIndex+12));

  setConnect(2,Origin+X*(outerGap+outerWallThick)+Y*(length/2.0),-X);
  setLinkSurf(2,-SMap.realSurf(buildIndex+13));

  setConnect(3,Origin+X*(ringGap+ringWallThick)+Y*(segmentLength/2.0),X);
  setLinkSurf(3,SMap.realSurf(buildIndex+14));

  
  setConnect(5,Origin-Z*(floorThick+floorDepth)+Y*(length/2.0),-Z);
  setLinkSurf(5,-SMap.realSurf(buildIndex+15));

  setConnect(5,Origin+Z*(roofThick+roofHeight)+Y*(length/2.0),Z);
  setLinkSurf(5,SMap.realSurf(buildIndex+16));

  const Geometry::Quaternion Qz=
    Geometry::Quaternion::calcQRotDeg(-segmentAngle,Z);
  Geometry::Vec3D XAxis(X);
  Geometry::Vec3D YAxis(Y);
  Qz.rotate(XAxis);
  Qz.rotate(YAxis);

  Geometry::Vec3D RPoint(Origin+X*ringGap+Y*segmentLength);

  const double midDist=0.5*(length-segmentLength)*
    cos(M_PI*segmentAngle/180.0);
  
  setConnect(7,Origin+X*(ringGap+ringWallThick)+
	     Y*(segmentLength)+
	     YAxis*midDist,
	     XAxis);
  setLinkSurf(7,SMap.realSurf(buildIndex+114));

  // Connect point is the place that the main ring conects to
  // calculated

  const double L1=length+frontWallThick-segmentLength;
  const Geometry::Vec3D newBeamPt=Origin+
    Y*(length+frontWallThick)+
    X*(L1*sin(M_PI*segmentAngle/180.0));

  setConnect(9,newBeamPt,YAxis);
  setLinkSurf(9,SMap.realSurf(buildIndex+12));

  return;
}

void
FrontEndCave::createAll(Simulation& System,
		       const attachSystem::FixedComp& FC,
		       const long int FIndex)
  /*!
    Generic function to create everything
    \param System :: Simulation item
    \param FC :: FixedComp
    \param FIndex :: Fixed Index
  */
{
  ELog::RegMethod RegA("FrontEndCave","createAll(FC)");

  populate(System.getDataBase());
  createUnitVector(FC,FIndex);
  
  createSurfaces();    
  createObjects(System);
  
  createLinks();
  insertObjects(System);   
  
  return;
}
  
}  // NAMESPACE essSystem