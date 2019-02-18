/*********************************************************************
  CombLayer : MCNP(X) Input builder

 * File:   essBuild/Linac.cxx
 *
 * Copyright (c) 2017-2018 by Konstantin Batkov
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
#include "BaseVisit.h"
#include "BaseModVisit.h"
#include "support.h"
#include "stringCombine.h"
#include "MatrixBase.h"
#include "Matrix.h"
#include "Vec3D.h"
#include "Quaternion.h"
#include "Surface.h"
#include "surfRegister.h"
#include "objectRegister.h"
#include "surfEqual.h"
#include "Quadratic.h"
#include "Plane.h"
#include "Cylinder.h"
#include "Line.h"
#include "Rules.h"
#include "varList.h"
#include "Code.h"
#include "FuncDataBase.h"
#include "inputParam.h"
#include "HeadRule.h"
#include "Object.h"
#include "groupRange.h"
#include "objectGroups.h"
#include "Simulation.h"
#include "ReadFunctions.h"
#include "ModelSupport.h"
#include "MaterialSupport.h"
#include "generateSurf.h"
#include "LinkUnit.h"
#include "FixedComp.h"
#include "FixedOffset.h"
#include "ContainedComp.h"
#include "BaseMap.h"
#include "FixedOffset.h"
#include "surfDBase.h"
#include "surfDIter.h"
#include "surfDivide.h"
#include "SurInter.h"
#include "mergeTemplate.h"
#include "World.h"
#include "AttachSupport.h"

#include "CellMap.h"
#include "BeamDump.h"
#include "FaradayCup.h"
#include "CopiedComp.h"
#include "DTL.h"
#include "DTLArray.h"
#include "TSW.h"
#include "Linac.h"

namespace essSystem
{

Linac::Linac(const std::string& Key)  :
  attachSystem::ContainedComp(),
  attachSystem::FixedOffset(Key,17), attachSystem::CellMap(),
  beamDump(new BeamDump(Key,"BeamDump")),
  fc4(new FaradayCup(Key,"FC4")),
  dtl(new DTLArray(Key,"DTLArray"))
  /*!
    Constructor BUT ALL variable are left unpopulated.
    \param Key :: Name for item in search
  */
{
  ELog::RegMethod RegA("Linac","Linac(const std::string&)");
  ModelSupport::objectRegister& OR = ModelSupport::objectRegister::Instance();
  OR.addObject(beamDump);
  OR.addObject(fc4);
  OR.addObject(dtl);
}

Linac::Linac(const Linac& A) :
  attachSystem::ContainedComp(A),
  attachSystem::FixedOffset(A),
  attachSystem::CellMap(A),
  engActive(A.engActive),
  lengthBack(A.lengthBack),
  lengthFront(A.lengthFront),
  widthLeft(A.widthLeft),
  widthRight(A.widthRight),
  height(A.height),
  depth(A.depth),
  wallThick(A.wallThick),
  roofThick(A.roofThick),
  floorThick(A.floorThick),
  floorWidthLeft(A.floorWidthLeft),
  floorWidthRight(A.floorWidthRight),
  nAirLayers(A.nAirLayers),
  airMat(A.airMat),wallMat(A.wallMat),
  nTSW(A.nTSW),
  nStubs(A.nStubs),
  nDTL(A.nDTL),
  beamDump(new BeamDump(*A.beamDump)),
  fc4(new FaradayCup(*A.fc4)),
  dtl(A.dtl)
  /*!
    Copy constructor
    \param A :: Linac to copy
  */
{}

Linac&
Linac::operator=(const Linac& A)
  /*!
    Assignment operator
    \param A :: Linac to copy
    \return *this
  */
{
  if (this!=&A)
    {
      attachSystem::ContainedComp::operator=(A);
      attachSystem::FixedOffset::operator=(A);
      attachSystem::CellMap::operator=(A);
      engActive=A.engActive;
      lengthBack=A.lengthBack;
      lengthFront=A.lengthFront;
      widthLeft=A.widthLeft;
      widthRight=A.widthRight;
      height=A.height;
      depth=A.depth;
      wallThick=A.wallThick;
      roofThick=A.roofThick;
      floorThick=A.floorThick;
      floorWidthLeft=A.floorWidthLeft;
      floorWidthRight=A.floorWidthRight;
      nAirLayers=A.nAirLayers;
      airMat=A.airMat;
      wallMat=A.wallMat;
      nTSW=A.nTSW;
      nStubs=A.nStubs;
      nDTL=A.nDTL;
      *beamDump=*A.beamDump;
      *fc4=*A.fc4;
      *dtl=*A.dtl;
    }
  return *this;
}

Linac::~Linac()
  /*!
    Destructor
  */
{}

void
Linac::populate(const FuncDataBase& Control)
  /*!
    Populate all the variables
    \param Control :: Variable data base
  */
{
  ELog::RegMethod RegA("Linac","populate");

  FixedOffset::populate(Control);
  engActive=Control.EvalPair<int>(keyName,"","EngineeringActive");

  lengthBack=Control.EvalVar<double>(keyName+"LengthBack");
  lengthFront=Control.EvalVar<double>(keyName+"LengthFront");
  widthLeft=Control.EvalVar<double>(keyName+"WidthLeft");
  widthRight=Control.EvalVar<double>(keyName+"WidthRight");
  height=Control.EvalVar<double>(keyName+"Height");
  depth=Control.EvalVar<double>(keyName+"Depth");
  wallThick=Control.EvalVar<double>(keyName+"WallThick");
  roofThick=Control.EvalVar<double>(keyName+"RoofThick");
  floorThick=Control.EvalVar<double>(keyName+"FloorThick");
  floorWidthLeft=Control.EvalVar<double>(keyName+"FloorWidthLeft");
  floorWidthRight=Control.EvalVar<double>(keyName+"FloorWidthRight");
  nAirLayers=Control.EvalDefVar<size_t>(keyName+"NAirLayers", 1);

  airMat=ModelSupport::EvalMat<int>(Control,keyName+"AirMat");
  wallMat=ModelSupport::EvalMat<int>(Control,keyName+"WallMat");
  nTSW=Control.EvalVar<size_t>(keyName+"NTSW");
  nStubs=Control.EvalDefVar<size_t>(keyName+"NStubs",2);
  nDTL=Control.EvalDefVar<size_t>(keyName+"NDTLTanks",5);

  return;
}

void
Linac::createUnitVector(const attachSystem::FixedComp& FC,
			const long int sideIndex)
  /*!
    Create the unit vectors
    \param FC :: object for origin
    \param sideIndex :: sideIndex
  */
{
  ELog::RegMethod RegA("Linac","createUnitVector");

  FixedComp::createUnitVector(FC,sideIndex);
  applyOffset();

  return;
}

void
Linac::layerProcess(Simulation& System, const std::string& cellName,
		    const long int& lpS, const long int& lsS,
		    const size_t& nLayers, const int& mat)
  /*!
    Processes the splitting of the surfaces into a multilayer system
    \param System :: Simulation to work on
    \param cellName :: TSW wall cell name
    \param lpS :: link pont of primary surface
    \param lsS :: link point of secondary surface
    \param nLayers :: number of layers to divide to
    \param mat :: material
  */
  {
    ELog::RegMethod RegA("Linac","layerProcess");
    if (nLayers>1)
      {
	const int pS = getLinkSurf(lpS);
	const int sS = getLinkSurf(lsS);

	const attachSystem::CellMap* CM = dynamic_cast<const attachSystem::CellMap*>(this);
	MonteCarlo::Object* wallObj(0);
	int wallCell(0);

	if (CM)
	  {
	    wallCell=CM->getCell(cellName);
	    wallObj=System.findObject(wallCell);
	  }

	if (!wallObj)
	  throw ColErr::InContainerError<int>(wallCell,
					      "Cell '" + cellName + "' not found");

	double baseFrac = 1.0/nLayers;
	ModelSupport::surfDivide DA;
	for(size_t i=1;i<nLayers;i++)
	  {
	    DA.addFrac(baseFrac);
	    DA.addMaterial(mat);
	    baseFrac += 1.0/nLayers;
	  }
	DA.addMaterial(mat);

	DA.setCellN(wallCell);
	DA.setOutNum(cellIndex, buildIndex+10000);

	ModelSupport::mergeTemplate<Geometry::Plane,
				    Geometry::Plane> surroundRule;

	surroundRule.setSurfPair(SMap.realSurf(pS),
				 SMap.realSurf(sS));

	std::string OutA = getLinkString(lpS);
	std::string OutB = getLinkString(-lsS);

	surroundRule.setInnerRule(OutA);
	surroundRule.setOuterRule(OutB);

	DA.addRule(&surroundRule);
	DA.activeDivideTemplate(System);

	cellIndex=DA.getCellNum();
      }
  }


void
Linac::buildTSW(Simulation& System) const
/*!
  Build Temporary shielding walls
*/
{
  ELog::RegMethod RegA("Linac","buildTSW");

  ModelSupport::objectRegister& OR=ModelSupport::objectRegister::Instance();

  for (size_t i=0; i<nTSW; i++)
    {
      std::shared_ptr<TSW>
        wall(new TSW(keyName,"TSW",i));
      OR.addObject(wall);
      // addInsertCell would not work since sometimes we split the air cell by layerProccess
      // wall->addInsertCell(this->getCells("air"));
      // instead, we call addToInsertControl

      wall->createAll(System,*this,13,14,15,16);
      attachSystem::addToInsertControl(System,*this,*wall);
    }
  return;
}


void
Linac::createSurfaces()
  /*!
    Create All the surfaces
  */
{
  ELog::RegMethod RegA("Linac","createSurfaces");

  //  ModelSupport::buildIndex& SurI=ModelSupport::buildIndex::Instance();

  // Redefine the outer void boundary sphere since the default one is too small
  //  SurI.createSurface(1,"so 60000"); use updateSurface when SA has it implemented. Now change World.cxx

  ModelSupport::buildPlane(SMap,buildIndex+1,Origin-Y*(lengthBack),Y);
  ModelSupport::buildPlane(SMap,buildIndex+2,Origin+Y*(lengthFront),Y);

  ModelSupport::buildPlane(SMap,buildIndex+3,Origin-X*(widthRight),X);
  ModelSupport::buildPlane(SMap,buildIndex+4,Origin+X*(widthLeft),X);

  ModelSupport::buildPlane(SMap,buildIndex+5,Origin-Z*(depth),Z);
  ModelSupport::buildPlane(SMap,buildIndex+6,Origin+Z*(height),Z);

  ModelSupport::buildPlane(SMap,buildIndex+12,Origin+Y*(lengthFront+wallThick),Y);

  ModelSupport::buildPlane(SMap,buildIndex+13,Origin-X*(widthRight+wallThick),X);
  ModelSupport::buildPlane(SMap,buildIndex+14,Origin+X*(widthLeft+wallThick),X);
  // floor
  ModelSupport::buildPlane(SMap,buildIndex+23,Origin-X*(floorWidthRight),X);
  ModelSupport::buildPlane(SMap,buildIndex+24,Origin+X*(floorWidthLeft),X);

  ModelSupport::buildPlane(SMap,buildIndex+15,Origin-Z*(depth+floorThick),Z);
  ModelSupport::buildPlane(SMap,buildIndex+16,Origin+Z*(height+roofThick),Z);

  return;
}

void
Linac::createObjects(Simulation& System)
  /*!
    Adds the all the components
    \param System :: Simulation to create objects in
  */
{
  ELog::RegMethod RegA("Linac","createObjects");

  std::string Out;
  Out=ModelSupport::getComposite(SMap,buildIndex," 1 -2 3 -4 5 -6 ");
  System.addCell(MonteCarlo::Object(cellIndex++,airMat,0.0,Out));
  setCell("air", cellIndex-1);

  // side walls and roof
  Out=ModelSupport::getComposite(SMap,buildIndex,
				 " 1 -12 13 -14 5 -16 (-1:2:-3:4:6) ");
  System.addCell(MonteCarlo::Object(cellIndex++,wallMat,0.0,Out));
  // wall bottom slab
  Out=ModelSupport::getComposite(SMap,buildIndex," 1 -12 23 -24 15 -5 ");
  System.addCell(MonteCarlo::Object(cellIndex++,wallMat,0.0,Out));

  layerProcess(System, "air", 11, 12, nAirLayers, airMat);

  Out=ModelSupport::getComposite(SMap,buildIndex," (1 -12 13 -14 15 -16) : (1 -12 23 -24 15 -5) ");
  addOuterSurf(Out);

  return;
}


void
Linac::createLinks()
  /*!
    Create all the linkes
  */
{
  ELog::RegMethod RegA("Linac","createLinks");

  // outer links
  FixedComp::setConnect(0,Origin-Y*(lengthBack),-Y);
  FixedComp::setLinkSurf(0,-SMap.realSurf(buildIndex+1));

  FixedComp::setConnect(1,Origin+Y*(lengthFront+wallThick),Y);
  FixedComp::setLinkSurf(1,SMap.realSurf(buildIndex+12));

  FixedComp::setConnect(2,Origin-X*(widthRight+wallThick),-X);
  FixedComp::setLinkSurf(2,-SMap.realSurf(buildIndex+13));

  FixedComp::setConnect(3,Origin+X*(widthLeft+wallThick),X);
  FixedComp::setLinkSurf(3,SMap.realSurf(buildIndex+14));

  FixedComp::setConnect(4,Origin-Z*(depth+floorThick),-Z);
  FixedComp::setLinkSurf(4,-SMap.realSurf(buildIndex+15));

  FixedComp::setConnect(5,Origin+Z*(height+roofThick),Z);
  FixedComp::setLinkSurf(5,SMap.realSurf(buildIndex+16));

  // walls
  FixedComp::setConnect(10,Origin-Y*(lengthBack),Y);
  FixedComp::setLinkSurf(10,SMap.realSurf(buildIndex+1));

  FixedComp::setConnect(11,Origin+Y*(lengthFront),Y); // should be negative, but layerProcess needs positive
  FixedComp::setLinkSurf(11,SMap.realSurf(buildIndex+2));

  FixedComp::setConnect(12,Origin-X*(widthRight),X);
  FixedComp::setLinkSurf(12,SMap.realSurf(buildIndex+3));

  FixedComp::setConnect(13,Origin+X*(widthLeft),-X); // check left/right/location
  FixedComp::setLinkSurf(13,-SMap.realSurf(buildIndex+4));

  FixedComp::setConnect(14,Origin-Z*(depth),Z);
  FixedComp::setLinkSurf(14,SMap.realSurf(buildIndex+5));

  FixedComp::setConnect(15,Origin+Z*(height),-Z);
  FixedComp::setLinkSurf(15,-SMap.realSurf(buildIndex+6));

  return;
}




void
Linac::createAll(Simulation& System,
		 const attachSystem::FixedComp& FC,
		 const long int sideIndex)
  /*!
    Generic function to create everything
    \param System :: Simulation item
    \param FC :: Central origin
    \param sideIndex :: link point
  */
{
  ELog::RegMethod RegA("Linac","createAll");

  populate(System.getDataBase());
  createUnitVector(FC,sideIndex);
  createSurfaces();
  createLinks();
  createObjects(System);
  insertObjects(System);


  if (nDTL>0)
    {
      dtl->createAll(System,World::masterOrigin(),0);
      attachSystem::addToInsertControl(System,*this,*dtl);

      // Beam dump and Faraday Cup are built with respect to the end of last DTL
      const long int backLP(static_cast<long int>(dtl->NConnect()-4));

      beamDump->createAll(System,*dtl,backLP);
      attachSystem::addToInsertControl(System,*this,*beamDump);

      fc4->createAll(System,*dtl,backLP);
      attachSystem::addToInsertControl(System,*this,*fc4);
    }
  buildTSW(System);

  return;
}

}  // essSystem essSystem
