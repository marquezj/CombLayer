/********************************************************************* 
  CombLayer : MCNP(X) Input builder
 
 * File:   construct/CylMod.cxx
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


#include "Exception.h"
#include "FileReport.h"
#include "GTKreport.h"
#include "NameStack.h"
#include "RegMethod.h"
#include "OutputLog.h"
#include "surfRegister.h"
#include "objectRegister.h"
#include "BaseVisit.h"
#include "BaseModVisit.h"
#include "MatrixBase.h"
#include "Matrix.h"
#include "Vec3D.h"
#include "Quaternion.h"
#include "Surface.h"
#include "surfIndex.h"
#include "Quadratic.h"
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
#include "MaterialSupport.h"
#include "generateSurf.h"
#include "support.h"
#include "LinkUnit.h"
#include "FixedComp.h"
#include "FixedOffset.h"
#include "ContainedComp.h"
#include "LayerComp.h"
#include "BaseMap.h"
#include "CellMap.h"
#include "ConicInfo.h"
#include "WedgeInsert.h"
#include "ModBase.h"
#include "CylMod.h"

namespace constructSystem
{

CylMod::CylMod(const std::string& Key) :
  constructSystem::ModBase(Key,6),
  mainCell(0)
  /*!
    Constructor
    \param Key :: Name of construction key
  */
{}

CylMod::CylMod(const CylMod& A) : 
  constructSystem::ModBase(A),
  radius(A.radius),height(A.height),mat(A.mat),temp(A.temp),
  nConic(A.nConic),Conics(A.Conics),nWedge(A.nWedge),
  Wedges(A.Wedges),mainCell(A.mainCell)
  /*!
    Copy constructor
    \param A :: CylMod to copy
  */
{}

CylMod&
CylMod::operator=(const CylMod& A)
  /*!
    Assignment operator
    \param A :: CylMod to copy
    \return *this
  */
{
  if (this!=&A)
    {
      constructSystem::ModBase::operator=(A);
      radius=A.radius;
      height=A.height;
      mat=A.mat;
      temp=A.temp;
      nConic=A.nConic;
      Conics=A.Conics;
      nWedge=A.nWedge;
      Wedges=A.Wedges;
      mainCell=A.mainCell;
    }
  return *this;
}

CylMod::~CylMod()
  /*!
    Destructor
  */
{}

CylMod*
CylMod::clone() const
  /*!
    Clone copy constructor
    \return copy of this
  */
{
  return new CylMod(*this);
}

void
CylMod::populate(const FuncDataBase& Control)
  /*!
    Populate all the variables
    \param Control :: Variable table to use
  */
{
  ELog::RegMethod RegA("CylMod","populate");

  FixedOffset::populate(Control);
  
  double R,H,T;
  int M;
  nLayers=Control.EvalVar<size_t>(keyName+"NLayers");
  if (nLayers==0) nLayers=1;  // Layers include midle

  for(size_t i=0;i<nLayers;i++)
    {
      if (i)
	{
	  const std::string IStr(std::to_string(i));
	  H+=2.0*Control.EvalVar<double>(keyName+"HGap"+IStr);   
	  R+=Control.EvalVar<double>(keyName+"RadGap"+IStr);   
	  M=ModelSupport::EvalMat<int>(Control,keyName+"Material"+IStr);   
	  T=(!M) ? 0.0 : 
	    Control.EvalVar<double>(keyName+"Temp"+IStr); 
	}
      else
	{
	  H=Control.EvalVar<double>(keyName+"Height");   
	  R=Control.EvalVar<double>(keyName+"Radius");   
	  M=ModelSupport::EvalMat<int>(Control,keyName+"Mat");   
	  T=Control.EvalVar<double>(keyName+"Temp");   
	}
      radius.push_back(R);
      height.push_back(H);
      mat.push_back(M);
      temp.push_back(T);
    }

  nConic=Control.EvalVar<size_t>(keyName+"NConic");
  for(size_t i=0;i<nConic;i++)
    {
      const std::string KN=keyName+"Conic"+std::to_string(i+1);
      const Geometry::Vec3D C=
	Control.EvalVar<Geometry::Vec3D>(KN+"Cent");
      const Geometry::Vec3D A=
	Control.EvalVar<Geometry::Vec3D>(KN+"Axis");
      const double W=
	Control.EvalVar<double>(KN+"Wall");
      const int WM=
	ModelSupport::EvalMat<int>(Control,KN+"WallMat");
      const int M=
	ModelSupport::EvalMat<int>(Control,KN+"Mat");
      const double ang=
	Control.EvalVar<double>(KN+"Angle");
      Conics.push_back(ConicInfo(C,A,ang,M,W,WM));
    }

  nWedge=Control.EvalVar<size_t>(keyName+"NWedge");  
  return;
}

void
CylMod::createSurfaces()
  /*!
    Create planes for the silicon and Polyethene layers
  */
{
  ELog::RegMethod RegA("CylMod","createSurfaces");

  // Divide plane
  ModelSupport::buildPlane(SMap,buildIndex+1,Origin,X);  
  ModelSupport::buildPlane(SMap,buildIndex+2,Origin,Y);  

  int SI(buildIndex);
  for(size_t i=0;i<nLayers;i++)
    {
      ModelSupport::buildCylinder(SMap,SI+7,Origin,Z,radius[i]);  
      ModelSupport::buildPlane(SMap,SI+5,Origin-Z*height[i]/2.0,Z);  
      ModelSupport::buildPlane(SMap,SI+6,Origin+Z*height[i]/2.0,Z);  
      SI+=10;
    }
  // CONICS
  SI=buildIndex+500;
  for(size_t i=0;i<nConic;i++)
    {
      const Geometry::Vec3D Pt=Conics[i].getCent(X,Y,Z);
      const Geometry::Vec3D CY=Conics[i].getAxis(X,Y,Z);
      const int cylFlag=Conics[i].isCyl();
      const double rangle=Conics[i].getAngle();
      ModelSupport::buildPlane(SMap,SI+1,Origin+Pt,CY);
      if (!cylFlag)
	{
	  const double wallThick=Conics[i].getWall()/cos(M_PI*rangle/180.0);
	  // Note dividing surface
	  ModelSupport::buildCone(SMap,SI+7,Origin+Pt,CY,rangle);
	  if (Conics[i].getWall()>Geometry::zeroTol)
	    {
	      ModelSupport::buildCone(SMap,SI+17,
				      Origin+Pt-CY*wallThick,CY,rangle);
	      ModelSupport::buildPlane(SMap,SI+11,Origin+Pt-CY*wallThick,CY);
	    }
	}
      else
	{
	  const double wallThick=Conics[i].getWall();
	  ModelSupport::buildCylinder(SMap,SI+7,Origin+Pt,CY,rangle);
	  ModelSupport::buildPlane(SMap,SI+1,Origin+Pt,CY);
	  if (Conics[i].getWall()>Geometry::zeroTol)
	    {
	      ModelSupport::buildCylinder(SMap,SI+17,Origin+Pt,CY,
					  rangle+wallThick);
	      ModelSupport::buildPlane(SMap,SI+11,Origin+Pt-CY*wallThick,CY);
	    }
	}

      SI+=100;
    }

  return; 
}

void
CylMod::createObjects(Simulation& System)
  /*!
    Create the vaned moderator
    \param System :: Simulation to add results
  */
{
  ELog::RegMethod RegA("CylMod","createObjects");

  std::string Out;
  // First make conics:
  int CI(buildIndex+500);
  HeadRule OutUnit;

  for(size_t i=0;i<nConic;i++)
    {
      Out=ModelSupport::getComposite(SMap,buildIndex,CI," -7 5 -6 -7M 1M");
      System.addCell(MonteCarlo::Object(cellIndex++,
				       Conics[i].getMat(),temp[0],Out));
      if (Conics[i].getWall()>Geometry::zeroTol)
	{
	  Out=ModelSupport::getComposite(SMap,buildIndex,CI,
					 " -7 5 -6 (7M:-1M) -17M 11M");
	  System.addCell(MonteCarlo::Object(cellIndex++,
					   Conics[i].getWallMat(),
					   temp[0],Out));
	  Out=ModelSupport::getComposite(SMap,buildIndex,CI," -7 5 -6 -17M 11M ");
	}
      OutUnit.addUnion(Out);
      CI+=100;
    }
  OutUnit.makeComplement();

  mainCell=cellIndex;
  int SI(buildIndex);
  for(size_t i=0;i<nLayers;i++)
    {
      Out=ModelSupport::getComposite(SMap,SI," -7 5 -6 ");

      if ((i+1)==nLayers) addOuterSurf(Out);
      if (i)
	Out+=ModelSupport::getComposite(SMap,SI-10," (7:-5:6) ");
      else
	Out+=OutUnit.display();
      System.addCell(MonteCarlo::Object(cellIndex++,mat[i],temp[i],Out));
      CellMap::setCell(keyName,i,cellIndex-1);
      SI+=10;
    }
  return; 
}

void
CylMod::createLinks()
  /*!
    Creates a full attachment set
  */
{  
  ELog::RegMethod RegA("CylMod","createLinks");

  if (!nLayers) return;
  const size_t NL(nLayers-1);
  const int SI(buildIndex+static_cast<int>(NL)*10);
  
  FixedComp::setConnect(0,Origin-Y*radius[NL],-Y);
  FixedComp::setLinkSurf(0,SMap.realSurf(SI+7));
  FixedComp::setBridgeSurf(0,-SMap.realSurf(buildIndex+2));
  
  FixedComp::setConnect(1,Origin+Y*radius[NL],Y);
  FixedComp::setLinkSurf(1,SMap.realSurf(SI+7));
  FixedComp::setBridgeSurf(1,SMap.realSurf(buildIndex+2));

  FixedComp::setConnect(2,Origin-X*radius[NL],-X);
  FixedComp::setLinkSurf(2,SMap.realSurf(SI+7));
  FixedComp::setBridgeSurf(2,-SMap.realSurf(buildIndex+1));
  
  FixedComp::setConnect(3,Origin+X*radius[NL],X);
  FixedComp::setLinkSurf(3,SMap.realSurf(SI+7));
  FixedComp::setBridgeSurf(3,SMap.realSurf(buildIndex+1));
      
  FixedComp::setConnect(4,Origin-Z*(height[NL]/2.0),-Z);
  FixedComp::setLinkSurf(4,-SMap.realSurf(SI+5));
  
  FixedComp::setConnect(5,Origin+Z*(height[NL]/2.0),Z);
  FixedComp::setLinkSurf(5,SMap.realSurf(SI+6));

  return;
}


Geometry::Vec3D
CylMod::getSurfacePoint(const size_t layerIndex,
			const long int sideIndex) const
  /*!
    Given a side and a layer calculate the link point
    \param sideIndex :: Side [0-5]
    \param layerIndex :: layer, 0 is inner moderator [0-6]
    \return Surface point
  */
{
  ELog::RegMethod RegA("CylMod","getSurfacePoint");

  const size_t SI((sideIndex>0) ?
                  static_cast<size_t>(sideIndex-1) :
                  static_cast<size_t>(-1-sideIndex));
  
  if (layerIndex>=nLayers) 
    throw ColErr::IndexError<size_t>(layerIndex,nLayers,"layer");

  // Modification map:
  switch(SI)
    {
    case 0:
      return Origin-Y*radius[layerIndex];
    case 1:
      return Origin+Y*radius[layerIndex];
    case 2:
      return Origin-X*radius[layerIndex];
    case 3:
      return Origin+X*radius[layerIndex];
    case 4:
      return Origin-Z*(height[layerIndex]/2.0);
    case 5:
      return Origin+Z*(height[layerIndex]/2.0);
    }
  throw ColErr::IndexError<long int>(sideIndex,6,"sideIndex ");
}

int
CylMod::getCommonSurf(const long int sideIndex) const
  /*!
    Given a side calculate the boundary surface
    \param sideIndex :: Side [1-6]
    \return Common dividing surface [outward pointing]
  */
{
  ELog::RegMethod RegA("CylModerator","getCommonSurf");

  switch(std::abs(sideIndex))
    {
    case 1:
      return -SMap.realSurf(buildIndex+2);
    case 2:
      return SMap.realSurf(buildIndex+2);
    case 3:
      return -SMap.realSurf(buildIndex+1);
    case 4:
      return SMap.realSurf(buildIndex+1);
    case 5:
    case 6:
      return 0;
    }
  throw ColErr::IndexError<long int>(sideIndex,6,"sideIndex ");
}

std::string
CylMod::getLayerString(const size_t layerIndex,
		       const long int sideIndex) const
  /*!
    Given a side and a layer calculate the link surf
    \param layerIndex :: layer, 0 is inner moderator [0-4]
    \param sideIndex :: Side [0-5]
    \return Surface string
  */
{
  ELog::RegMethod RegA("CylMod","getLayerString");

  if (layerIndex>=nLayers) 
    throw ColErr::IndexError<size_t>(layerIndex,nLayers,"layer");

  const int NL(static_cast<int>(layerIndex));
  const int SI(buildIndex+NL*10);
  HeadRule HR;
  switch(std::abs(sideIndex))
    {
    case 1:
    case 2:
    case 3:
    case 4:
      HR.addIntersection(SMap.realSurf(SI+7));
      break;
    case 5:
      HR.addIntersection(-SMap.realSurf(SI+5));
      break;
    case 6:
      HR.addIntersection(SMap.realSurf(SI+6));
      break;
    }
  if (sideIndex<0)
    HR.makeComplement();
  return HR.display();
}

int
CylMod::getLayerSurf(const size_t layerIndex,
		     const long int sideIndex) const
  /*!
    Given a side and a layer calculate the link surf. Surf points out
    \param sideIndex :: Side [0-5]
    \param layerIndex :: layer, 0 is inner moderator [0-4]
    \return Surface number [outgoing]
  */
{
  ELog::RegMethod RegA("CylMod","getLayerSurf");

  if (layerIndex>=nLayers) 
    throw ColErr::IndexError<size_t>(layerIndex,nLayers,"layerIndex");
  
  const int SI(buildIndex+static_cast<int>(layerIndex)*10);
  switch(sideIndex)
    {
    case 1:
    case 2:
    case 3:
      return SMap.realSurf(SI+7);
    case 4:
      return -SMap.realSurf(SI+5);
    case 5:
      return SMap.realSurf(SI+6);
    case -1:
    case -2:
    case -3:
      return -SMap.realSurf(SI+7);
    case -4:
      return SMap.realSurf(SI+5);
    case -5:
      return -SMap.realSurf(SI+6);
    }
  throw ColErr::IndexError<long int>(sideIndex,6,"sideIndex");
}

void
CylMod::createWedges(Simulation& System)
  /*!
    Create Wedges
    \param System :: Simulation
   */
{
  ELog::RegMethod RegA("CylMod","createWedges");
  
  ModelSupport::objectRegister& OR=
    ModelSupport::objectRegister::Instance();

  for(size_t i=0;i<nWedge;i++)
    {
      WTYPE WPtr(new WedgeInsert(keyName+"Wedge",i+1));
      OR.addObject(WPtr);
      WPtr->addInsertCell(CellMap::getCell(keyName,0));
      WPtr->addInsertCell(CellMap::getCell(keyName,1));
      WPTr->setLayer(*this,1,1);      
      WPtr->createAll(System,*this,0);   // +ve Y direction [cylinder]
      Wedges.push_back(WPtr);
    }
  return;
}

void
CylMod::createAll(Simulation& System,
		  const attachSystem::FixedComp& orgFC,
		  const long int orgIndex,
		  const attachSystem::FixedComp& axisFC,
		  const long int sideIndex)
  /*!
    Extrenal build everything
    \param System :: Simulation
    \param axisFC :: FixedComponent for axis
    \param orgFC :: FixedComponent for origin
    \param sideIndex :: connection point 
   */
{
  ELog::RegMethod RegA("CylMod","createAll");
  populate(System.getDataBase());

  ModBase::createUnitVector(orgFC,orgIndex,axisFC,sideIndex);
  createSurfaces();
  createObjects(System);
  createLinks();
  insertObjects(System);       

  createWedges(System);
  return;
}

}  // NAMESPACE constructSystem
