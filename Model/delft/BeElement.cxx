/********************************************************************* 
  CombLayer : MCNP(X) Input builder
 
 * File:   delft/BeElement.cxx
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
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <complex>
#include <climits>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <memory>
#include <boost/multi_array.hpp>

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
#include "Surface.h"
#include "surfIndex.h"
#include "surfRegister.h"
#include "surfDIter.h"
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
#include "shutterBlock.h"
#include "Simulation.h"
#include "ModelSupport.h"
#include "generateSurf.h"
#include "LinkUnit.h"
#include "FixedComp.h"
#include "FixedOffset.h"
#include "ContainedComp.h"
#include "BaseMap.h"
#include "CellMap.h"
#include "SurfMap.h"
#include "LayerDivide1D.h"

#include "FuelLoad.h"
#include "ReactorGrid.h"
#include "RElement.h"
#include "BeElement.h"

namespace delftSystem
{


BeElement::BeElement(const size_t XI,const size_t YI,
		     const std::string& Key) :
  RElement(XI,YI,Key)
  /*!
    Constructor BUT ALL variable are left unpopulated.
    \param XI :: X index
    \param YI :: Y index
    \param Key :: KeyName
  */
{}

BeElement::BeElement(const BeElement& A) : 
  RElement(A),
  Width(A.Width),Depth(A.Depth),TopHeight(A.TopHeight),
  nLayer(A.nLayer),beMat(A.beMat)
  /*!
    Copy constructor
    \param A :: BeElement to copy
  */
{}
  
BeElement&
BeElement::operator=(const BeElement& A)
  /*!
    Assignment operator
    \param A :: BeElement to copy
    \return *this
  */
{
  if (this!=&A)
    {
      RElement::operator=(A);
      Width=A.Width;
      Depth=A.Depth;
      TopHeight=A.TopHeight;
      nLayer=A.nLayer;
      beMat=A.beMat;
    }
  return *this;
}

void
BeElement::populate(const FuncDataBase& Control)
  /*!
    Populate all the variables
    Requires that unset values are copied from previous block
    \param Control :: DataBase
  */
{
  ELog::RegMethod RegA("BeElement","populate");

  Width=ReactorGrid::getElement<double>
    (Control,keyName+"Width",XIndex,YIndex);
  Depth=ReactorGrid::getElement<double>
    (Control,keyName+"Depth",XIndex,YIndex);
  TopHeight=ReactorGrid::getElement<double>
    (Control,keyName+"TopHeight",XIndex,YIndex);

  beMat=ReactorGrid::getMatElement
    (Control,keyName+"Mat",XIndex,YIndex);

  nLayer=ReactorGrid::getElement<size_t>
    (Control,keyName+"NLayer",XIndex,YIndex);

  return;
}

void
BeElement::createUnitVector(const attachSystem::FixedComp& FC,
                            const Geometry::Vec3D& OG)
  /*!
    Create the unit vectors
    - Y from FC
    \param FC :: Reactor Grid Unit
    \param OG :: Origin
  */
{
  ELog::RegMethod RegA("BeElement","createUnitVector");

  attachSystem::FixedComp::createUnitVector(FC);
  Origin=OG;
  return;
}

void
BeElement::createSurfaces(const attachSystem::FixedComp& RG)
  /*!
    Creates/duplicates the surfaces for this block
    \param RG :: Reactor grid
  */
{  
  ELog::RegMethod RegA("BeElement","createSurface");

  // Planes [OUTER]:
  
  ModelSupport::buildPlane(SMap,surfIndex+1,Origin-Y*Depth/2.0,Y);
  ModelSupport::buildPlane(SMap,surfIndex+2,Origin+Y*Depth/2.0,Y); 
  ModelSupport::buildPlane(SMap,surfIndex+3,Origin-X*Width/2.0,X);
  ModelSupport::buildPlane(SMap,surfIndex+4,Origin+X*Width/2.0,X);
  ModelSupport::buildPlane(SMap,surfIndex+6,Z*TopHeight,Z);

  SMap.addMatch(surfIndex+5,RG.getSignedLinkSurf(5));

  return;
}

void
BeElement::createObjects(Simulation& System)
  /*!
    Create the objects
    \param System :: Simulation
  */
{
  ELog::RegMethod RegA("BeElement","createObjects");

  std::string Out;
  // Outer Layers
  Out=ModelSupport::getComposite(SMap,surfIndex," 1 -2 3 -4 5 -6 ");
  addOuterSurf(Out);      
  System.addCell(MonteCarlo::Qhull(cellIndex++,beMat,0.0,Out));
  addCell("Main",cellIndex-1);

  return;
}


void
BeElement::createLinks()
  /*!
    Creates a full attachment set
    0 - 1 standard points
    2 - 3 beamaxis points
  */
{

  return;
}

void
BeElement::layerProcess(Simulation& System,
			const FuelLoad& FuelSystem)
  /*!
    Layer all the be elements.
    Note all calls go to FuelSystem.getMaterial

    \param System :: Simulation
    \param FuelSystem :: Fuel Load system for materials
  */
{
  ELog::RegMethod RegA("BeElement","layerProcess");
  
  if (nLayer<2)
    {
      const int MatN=
	FuelSystem.getMaterial(XIndex+1,YIndex+1,0,1,beMat);
      if (MatN!=beMat)
	{
	  const int CN=getCell("Main");
	  MonteCarlo::Object* OPtr=System.findQhull(CN);
	  if (!OPtr)
	    throw ColErr::InContainerError<int>
	      (CN,"["+keyName+"] Main Be Cell");
	  
	  OPtr->setMaterial(MatN);
	}
      return;
    }

  ModelSupport::LayerDivide1D LD1(keyName+"Main");

  LD1.setSurfPair(SMap.realSurf(surfIndex+5),-SMap.realSurf(surfIndex+6));
  LD1.setFractions(nLayer);  

  std::vector<int> DefMat;
  for(size_t i=0;i<nLayer;i++)
    {
      const int MatN=
	FuelSystem.getMaterial(XIndex+1,YIndex+1,0,i+1,beMat);
      DefMat.push_back(MatN);
    }
  LD1.setMaterials(DefMat);
  LD1.divideCell(System,getCell("Main"));
  return;
}

void
BeElement::createAll(Simulation& System,
                     const attachSystem::FixedComp& RG,
		     const Geometry::Vec3D& OG,
		     const FuelLoad& FuelSystem)
  /*!
    Creation of the Be-Reflector unit
    \param System :: Simulation to add component to
    \param RG :: Fixed Unit
    \param OG :: Origin
    \param FuelSystem :: XML input form for material burnup
  */
{
  ELog::RegMethod RegA("BeElement","createAll");
  populate(System.getDataBase());

  createUnitVector(RG,OG);
  createSurfaces(RG);
  createObjects(System);
  createLinks();
  layerProcess(System,FuelSystem);
  insertObjects(System);       

  return;
}


} // NAMESPACE delftSystem
