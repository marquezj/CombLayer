/*********************************************************************
  CombLayer : MCNP(X) Input builder

 * File:   softimax/softimaxVariables.cxx
 *
 * Copyright (c) 2004-2020 by Stuart Ansell/Konstantin Batkov
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

#include "FileReport.h"
#include "NameStack.h"
#include "RegMethod.h"
#include "OutputLog.h"
#include "Vec3D.h"
#include "Code.h"
#include "varList.h"
#include "FuncDataBase.h"
#include "maxivVariables.h"

#include "CFFlanges.h"
#include "PipeGenerator.h"
#include "SplitPipeGenerator.h"
#include "BellowGenerator.h"
#include "BremCollGenerator.h"
#include "BremOpticsCollGenerator.h"
#include "BremMonoCollGenerator.h"
#include "LeadPipeGenerator.h"
#include "CrossGenerator.h"
#include "GateValveGenerator.h"
#include "JawValveGenerator.h"
#include "PipeTubeGenerator.h"
#include "PortTubeGenerator.h"
#include "PortItemGenerator.h"
#include "PipeShieldGenerator.h"
#include "VacBoxGenerator.h"
#include "GratingMonoGenerator.h"
#include "GratingUnitGenerator.h"
#include "TankMonoVesselGenerator.h"
#include "MonoBoxGenerator.h"
#include "FlangeMountGenerator.h"
#include "BeamPairGenerator.h"
#include "MirrorGenerator.h"
#include "CollGenerator.h"
#include "SqrFMaskGenerator.h"
#include "JawFlangeGenerator.h"
#include "PortChicaneGenerator.h"
#include "WallLeadGenerator.h"
#include "TwinPipeGenerator.h"
#include "DiffPumpGenerator.h"

namespace setVariable
{

namespace softimaxVar
{
  void undulatorVariables(FuncDataBase&,const std::string&);
  void frontMaskVariables(FuncDataBase&,const std::string&);
  void wallVariables(FuncDataBase&,const std::string&);
  void monoVariables(FuncDataBase&,const std::string&);

void
undulatorVariables(FuncDataBase& Control,
		     const std::string& undKey)
  /*!
    Builds the variables for the undulator
    \param Control :: Database
    \param undKey :: prename
  */
{
  ELog::RegMethod RegA("softimaxVariables[F]","undulatorVariables");

setVariable::PipeGenerator PipeGen;

  // Undulator length:
  // https://alfresco.maxiv.lu.se/share/page/site/bpo/document-details?nodeRef=workspace://SpacesStore/22a6b9ab-4f3b-4525-bd79-d4a4fd0be33d page 1
  constexpr double undulatorLen(390.55);
  constexpr double undulatorPipeLen(undulatorLen+8.0+13.95);
  PipeGen.setMat("Aluminium");
  PipeGen.setNoWindow();   // no window
  PipeGen.setCF<setVariable::CF63>();
  PipeGen.generatePipe(Control,undKey+"UPipe",-undulatorPipeLen/2.0,undulatorPipeLen);

  Control.addVariable(undKey+"UPipeWidth",6.0);
  Control.addVariable(undKey+"UPipeHeight",0.6);
  Control.addVariable(undKey+"UPipeFeThick",0.2);

  // undulator I Vacuum
  Control.addVariable(undKey+"UndulatorVGap",1.1);  // mininum 11mm
  Control.addVariable(undKey+"UndulatorLength",undulatorLen);
  Control.addVariable(undKey+"UndulatorMagnetWidth",6.0);
  Control.addVariable(undKey+"UndulatorMagnetDepth",3.0);
  Control.addVariable(undKey+"UndulatorSupportWidth",12.0);
  Control.addVariable(undKey+"UndulatorSupportThick",8.0);
  Control.addVariable(undKey+"UndulatorSupportLength",4.0);  // extra
  Control.addVariable(undKey+"UndulatorSupportVOffset",2.0);
  Control.addVariable(undKey+"UndulatorStandWidth",6.0);
  Control.addVariable(undKey+"UndulatorStandHeight",8.0);
  Control.addVariable(undKey+"UndulatorVoidMat","Void");
  Control.addVariable(undKey+"UndulatorMagnetMat","NbFeB");
  Control.addVariable(undKey+"UndulatorSupportMat","Copper");
  Control.addVariable(undKey+"UndulatorStandMat","Aluminium");

  return;
}

void
frontMaskVariables(FuncDataBase& Control,
		   const std::string& preName)
  /*!
    Variable for the front maste
    \param Control :: Database
    \param preName :: Beamline name
  */
{
  ELog::RegMethod RegA("softimaxVariables[F]","frontMaskVariables");

  const double FM1dist(1135.5);
  const double FM2dist(1624.2);

  setVariable::SqrFMaskGenerator CollGen;

  // dimensions are from softimax-description.djvu, page1

  // there are 2 ways to set FM variables, via angles or via front/back/min gaps
  // I have a drawing with gaps (softimax-description.djvu, page1), so I use the gap approach:
  // via angles:
  // FMaskGen.setFrontAngleSize(FM1dist,1300.0,1300.0);
  //  FMaskGen.setMinAngleSize(10.0,FM1dist,1000.0,1000.0);
  //  FMaskGen.setBackAngleSize(FM1dist, 1200.0,1100.0);
  // via gaps:
  CollGen.setCF<CF100>();
  CollGen.setFrontGap(3.99,1.97); // dy,dz
  CollGen.setBackGap(0.71,0.71); // dy,dz
  CollGen.setMinSize(10.0,0.71,0.71); // L,dy,dz
  CollGen.generateColl(Control,preName+"CollA",FM1dist,15.0);

  CollGen.setMinSize(25.0,0.71,0.71); // L,dy,dz
  CollGen.generateColl(Control,preName+"CollB",FM2dist,30.0);

  // move water pipes from centre because otherwise clip with aperature cone
  Control.addVariable(preName+"CollBPipeXWidth",5.0);

  return;
}

void
wallVariables(FuncDataBase& Control,
	      const std::string& wallKey)
/*!
    Set the variables for the frontEnd wall
    \param Control :: DataBase to use
    \param wallKey :: name before part names
  */
{
  ELog::RegMethod RegA("softimaxVariables[F]","wallVariables");

  WallLeadGenerator LGen;
  LGen.setWidth(70,140.0);
  LGen.generateWall(Control,wallKey,2.1);

  return;
}

void
monoVariables(FuncDataBase& Control,
	      const std::string& monoKey)
  /*!
    Builds the variables for the mono packge
    \param Control :: Database
    \param slitKey :: prename
  */
{
  ELog::RegMethod RegA("softimaxVariables[F]","monoVariables");

  //  setVariable::PipeGenerator PipeGen;
  setVariable::PortItemGenerator PItemGen;
  setVariable::TankMonoVesselGenerator MBoxGen;
  setVariable::GratingMonoGenerator MXtalGen;
  setVariable::GratingUnitGenerator MUnitGen;
  setVariable::FlangeMountGenerator FlangeGen;

  // ystep/width/height/depth/length
  //
  constexpr double zstep(1.4);
  MBoxGen.setCF<CF63>();   // set ports
  MBoxGen.setAFlange(10.2,1.0);
  MBoxGen.setBFlange(setVariable::CF63::flangeRadius,setVariable::CF63::flangeLength);
  MBoxGen.setPortLength(2.3,5.0);
  MBoxGen.generateBox(Control,monoKey+"MonoVessel",0.0,54.91,36.45,36.45); // ystep,R,height,depth
  Control.addVariable(monoKey+"MonoVesselOuterSize",63);
  Control.addVariable(monoKey+"MonoVesselPortBZStep",zstep);      // from primary: 131.4-130.0
  Control.addVariable(monoKey+"MonoVesselWallMat", "Aluminium");


  const std::string portName=monoKey+"MonoVessel";
  Control.addVariable(monoKey+"MonoVesselNPorts",1); // beam ports (lots!!)
  PItemGen.setCF<setVariable::CF120>(5.0);
  PItemGen.setPlate(setVariable::CF63::flangeLength,"SiO2");
  PItemGen.generatePort(Control,portName+"Port0",
			Geometry::Vec3D(0,5.0,0.0),
			Geometry::Vec3D(1,0,0));

  // crystals
  //  MXtalGen.generateGrating(Control,monoKey+"MonoXtal",0.0,3.0);
  // monounit
  constexpr double theta(1.0);
  MUnitGen.generateGrating(Control,monoKey+"Grating",0.0,theta); // yStep, angle
  Control.addVariable(monoKey+"GratingMirrorTheta",theta);
  Control.addVariable(monoKey+"GratingZLift",zstep);
  Control.addVariable(monoKey+"GratingMainBarDepth",1.5);

  FlangeGen.setNoPlate();
  FlangeGen.setBlade(8.0,15.5,1.0,0.0,"Copper",1);  // w,h,t,ang,active
  FlangeGen.generateMount(Control,monoKey+"ZeroOrderBlock",0);  // in beam
  Control.addVariable(monoKey+"ZeroOrderBlockZStep",50.0);
  Control.addVariable(monoKey+"ZeroOrderBlockBladeLift",zstep);
  Control.addVariable(monoKey+"ZeroOrderBlockHoleActive",1);
  Control.addVariable(monoKey+"ZeroOrderBlockHoleWidth",1.0);
  Control.addVariable(monoKey+"ZeroOrderBlockHoleHeight",1.0);


  return;
}

void
opticsHutVariables(FuncDataBase& Control,
		   const std::string& preName)
  /*!
    Variable for the main optics hutch walls
    \param Control :: Database
    \param preName :: Beamline name
  */
{
  ELog::RegMethod RegA("softimaxVariables[F]","opticsCaveVariables");

  const std::string hutName(preName+"OpticsHut");

  Control.addVariable(hutName+"Height",250.0);
  Control.addVariable(hutName+"Length",1070); // outer length, measured by KB - check
  Control.addVariable(hutName+"OutWidth",200.0);
  Control.addVariable(hutName+"RingWidth",75.0);
  Control.addVariable(hutName+"RingWallLen",80.0);
  Control.addVariable(hutName+"RingWallAngle",18.50);
  Control.addVariable(hutName+"RingConcThick",100.0);

  Control.addVariable(hutName+"InnerThick",0.3);
  Control.addVariable(hutName+"Extension",100.0);

  // Lead thicknesses are from 06643-03-000\ folio\ 1-2\ IND\ G.PDF
  Control.addVariable(hutName+"PbWallThick",1.6);
  Control.addVariable(hutName+"PbRoofThick",1.6);
  Control.addVariable(hutName+"PbBackThick",9);
  Control.addVariable(hutName+"PbFrontThick",2.0); // guess

  Control.addVariable(hutName+"OuterThick",0.3);

  Control.addVariable(hutName+"InnerOutVoid",10.0);  // side wall for chicane
  Control.addVariable(hutName+"OuterOutVoid",10.0);

  Control.addVariable(hutName+"SkinMat","Stainless304");
  Control.addVariable(hutName+"RingMat","Concrete");
  Control.addVariable(hutName+"PbMat","Lead");

  Control.addVariable(hutName+"InletXStep",0.0);
  Control.addVariable(hutName+"InletZStep",0.0);
  Control.addVariable(hutName+"InletRadius",5.0);

  Control.addVariable(hutName+"NChicane",2);

  PortChicaneGenerator PGen;
  PGen.setSize(8.0,80.0,45.0);
  PGen.generatePortChicane(Control,hutName+"Chicane0",320.0,-25.0);
  PGen.generatePortChicane(Control,hutName+"Chicane1",-350.0,-25.0);
  
  return;
}

void
m1MirrorVariables(FuncDataBase& Control,
		  const std::string& mirrorKey)
/*!
  Builds the variables for the M1 Mirror
  \param Control :: Database
  \param mirrorKey :: prename
*/
{
  ELog::RegMethod RegA("softimaxVariables[F]","m1MirrorVariables");

  setVariable::PipeTubeGenerator SimpleTubeGen;
  setVariable::MirrorGenerator MirrGen;
  setVariable::PipeGenerator PipeGen;

  const std::string frontName=mirrorKey+"M1TubeFront";
  PipeGen.setMat("Stainless304");
  PipeGen.setCF<CF63>();
  PipeGen.setBFlange(8.05,0.3);
  PipeGen.generatePipe(Control,frontName,0.0,7.6);
  Control.addVariable(frontName+"WindowActive",0);
  constexpr double xstep(2.2);
  Control.addVariable(frontName+"FlangeBackXStep",-xstep);

  ////////////////////////
  constexpr double theta = -1.0; // incident beam angle
  constexpr double phi = 0.0;   // rotation angle
  //  const double normialAngle=0.2;
  constexpr double vAngle=180.0;
  constexpr double centreDist(0.0); // along the beam line
  ////////////////////////

  const std::string mName=mirrorKey+"M1Tube";
  SimpleTubeGen.setCF<CF150>();
  SimpleTubeGen.generateTube(Control,mName,0.0,50.0);
  Control.addVariable(mName+"WallMat","Titanium");
  Control.addVariable(mName+"NPorts",0);   // beam ports

  // mirror in M1Tube
  constexpr double thick(6.0); // messured in .step
  MirrGen.setPlate(28.0, thick, 9.0);  //guess: length, thick, width
  constexpr double top(0.1);
  constexpr double depth(thick+1.0);
  constexpr double gap(0.5);
  constexpr double extra(1.0);
  MirrGen.setSupport(top, depth, gap, extra);
  MirrGen.setPrimaryAngle(0,vAngle,0);
  // x/y/z/theta/phi/radius
  MirrGen.generateMirror(Control,mirrorKey+"M1Mirror",
			 -xstep,
			 centreDist/2.0,
			 0.0,
			 theta,
			 phi,
			 0.0);
  Control.addVariable(mirrorKey+"M1MirrorYAngle",270.0); // to reflect horizontally

  Control.addVariable(mirrorKey+"M1StandHeight",110.0);
  Control.addVariable(mirrorKey+"M1StandWidth",30.0);
  Control.addVariable(mirrorKey+"M1StandLength",30.0);
  Control.addVariable(mirrorKey+"M1StandMat","SiO2");

  const std::string backName=mirrorKey+"M1TubeBack";
  PipeGen.setMat("Stainless304");
  PipeGen.setCF<CF63>();
  PipeGen.setAFlange(8.05,0.3);
  PipeGen.generatePipe(Control,backName,0.0,4.5); // yStep, length
  Control.addVariable(backName+"WindowActive",0);
  Control.addVariable(backName+"XYAngle",2*theta);
  Control.addVariable(backName+"XStep",xstep);
  Control.addVariable(backName+"FlangeFrontXStep",-xstep);

  return;
}

void
splitterVariables(FuncDataBase& Control,
		  const std::string& splitKey)
  /*!
    Builds the variables for the splitter at
    the end of the opticsHut/opticsBeam
    \param Control :: Database
    \param splitKey :: prename
  */
{
  ELog::RegMethod RegA("softimaxVariables[F]","splitVariables");
  setVariable::TwinPipeGenerator TwinGen;
  setVariable::BellowGenerator BellowGen;
  setVariable::GateValveGenerator GateGen;
  setVariable::PipeGenerator PipeGen;
  setVariable::PortTubeGenerator PTubeGen;
  setVariable::PortItemGenerator PItemGen;
  setVariable::PipeShieldGenerator ShieldGen;
  setVariable::PipeTubeGenerator SimpleTubeGen;
  setVariable::CollGenerator CollGen;


  constexpr double splitAngle(2.0);
  constexpr double splitLength(1.8);
  constexpr double splitXStep(3.96);
  TwinGen.setCF<CF50>();
  TwinGen.setJoinFlangeCF<CF150>();
  TwinGen.setAPos(-splitXStep,0);
  TwinGen.setBPos(splitXStep,0);
  TwinGen.setXYAngle(splitAngle,-splitAngle);
  TwinGen.generateTwin(Control,splitKey+"Splitter",0.0,splitLength);
  Control.addVariable(splitKey+"SplitterFlangeCJLength",0.2);
  Control.addVariable(splitKey+"SplitterFlangeARadius",4.0);
  Control.addVariable(splitKey+"SplitterFlangeBRadius",4.0);

  BellowGen.setCF<setVariable::CF50>();
  BellowGen.setFlangePair(setVariable::CF50::flangeRadius-0.4,
			  setVariable::CF50::flangeLength,
			  setVariable::CF50::flangeRadius,
			  setVariable::CF50::flangeLength);
  BellowGen.generateBellow(Control,splitKey+"BellowAA",0,16.0);

  Control.copyVarSet(splitKey+"BellowAA", splitKey+"BellowBA");


  const std::string m3PumpName=splitKey+"M3Pump";
  ELog::EM << "M3Pump: Close the caps" << ELog::endWarn;
  SimpleTubeGen.setCF<CF200>();
  SimpleTubeGen.setPipe(7.7, 0.3, 10.0, 2.0);
  SimpleTubeGen.generateTube(Control,m3PumpName,0.0,36.0);  // centre 13.5cm
  //  Control.addVariable(mName+"XStep",centreOffset);
  Control.addVariable(m3PumpName+"NPorts",4);   // beam ports

  const Geometry::Vec3D ZVec(0,0,1);
  constexpr double port0Length(5.95);
  PItemGen.setCF<setVariable::CF50>(port0Length);
  PItemGen.setPlate(0.0,"Void");
  PItemGen.generatePort(Control,m3PumpName+"Port0",Geometry::Vec3D(-5.02,0,0),ZVec);

  const Geometry::Vec3D ZVec2(-sin(splitAngle*2*M_PI/180),0,cos(splitAngle*2*M_PI/180));
  PItemGen.setCF<setVariable::CF50>(port0Length*cos(splitAngle*4*M_PI/180)+0.03);
  PItemGen.setPlate(0.0,"Void");
  PItemGen.generatePort(Control,m3PumpName+"Port1",Geometry::Vec3D(5.02,0,0),ZVec2);

  PItemGen.setCF<setVariable::CF40>(4.95);
  PItemGen.setPlate(0.0,"Void");
  PItemGen.generatePort(Control,m3PumpName+"Port2",Geometry::Vec3D(-4,0,0),-ZVec);

  PItemGen.setCF<setVariable::CF40>(4.95);
  PItemGen.setPlate(0.0,"Void");
  PItemGen.generatePort(Control,m3PumpName+"Port3",Geometry::Vec3D(4,0,0),-ZVec2);

  BellowGen.setCF<setVariable::CF40>();
  BellowGen.generateBellow(Control,splitKey+"BellowAB",0,15.5);

  PipeGen.setMat("Stainless304");
  PipeGen.setCF<CF40>();
  PipeGen.setBFlangeCF<CF40>();
  PipeGen.generatePipe(Control,splitKey+"JoinPipeAA",0.0,90.3);
  Control.addVariable(splitKey+"JoinPipeAAFlangeBackRadius", 6.7);
  Control.addVariable(splitKey+"JoinPipeAAFlangeBackLength", 1.0);
  Control.addVariable(splitKey+"JoinPipeAAWindowActive", 0);

  setVariable::BremOpticsCollGenerator OpticsCollGen;
  //OpticsCollGen.setCF<setVariable::CF63>();
  OpticsCollGen.setAperture(0.6,5);
  OpticsCollGen.setMaterial("Tungsten", "Stainless304");
  OpticsCollGen.setFlangeInnerRadius(setVariable::CF40::innerRadius,setVariable::CF40::innerRadius);
  OpticsCollGen.generateColl(Control,splitKey+"BremCollAA",0.0, 19.0);
  Control.addVariable(splitKey+"BremCollAAExtActive", 0);
  Control.addVariable(splitKey+"BremCollAAColLength", 8.0);
  Control.addVariable(splitKey+"BremCollAAInnerRadius", 5.7);
  Control.addVariable(splitKey+"BremCollAAColRadius", 3.5);
  Control.addVariable(splitKey+"BremCollAAWallThick", 1.0);
  Control.addVariable(splitKey+"BremCollAAFlangeARadius", 6.7);
  Control.addVariable(splitKey+"BremCollAAFlangeALength", 1.0);
  Control.addVariable(splitKey+"BremCollAAFlangeBRadius", 6.7);
  Control.addVariable(splitKey+"BremCollAAFlangeBLength", 1.0);


  PipeGen.setMat("Stainless304");
  PipeGen.setCF<CF40>();
  PipeGen.setBFlangeCF<CF40>();
  PipeGen.generatePipe(Control,splitKey+"JoinPipeAB",0.0,100.0);
  Control.addVariable(splitKey+"JoinPipeABFlangeFrontRadius", 6.7);
  Control.addVariable(splitKey+"JoinPipeABFlangeFrontLength", 1.0);
  Control.addVariable(splitKey+"JoinPipeABWindowActive", 0);

  // RIGHT BRANCH
  Control.copyVarSet(splitKey+"BellowAB", splitKey+"BellowBB");
  Control.copyVarSet(splitKey+"JoinPipeAA", splitKey+"JoinPipeBA");
  Control.copyVarSet(splitKey+"BremCollAA", splitKey+"BremCollBA");
  Control.copyVarSet(splitKey+"JoinPipeAB", splitKey+"JoinPipeBB");

  // ShieldGen.generateShield(Control,splitKey+"ScreenB",0.0,0.0);

  return;
}


void
m3MirrorVariables(FuncDataBase& Control,
		  const std::string& mirrorKey)
/*!
  Builds the variables for the M3 Mirror
  \param Control :: Database
  \param mirrorKey :: prename
*/
{
  ELog::RegMethod RegA("softimaxVariables[F]","m3MirrorVariables");

  setVariable::PipeTubeGenerator SimpleTubeGen;
  setVariable::MirrorGenerator MirrGen;
  setVariable::PipeGenerator PipeGen;

  PipeGen.setMat("Stainless304");
  PipeGen.setCF<CF63>();
  PipeGen.setBFlangeCF<CF150>();
  PipeGen.generatePipe(Control,mirrorKey+"M3Front",0.0,4.05);
  Control.addVariable(mirrorKey+"M3FrontWindowActive",0);
  Control.addVariable(mirrorKey+"M3FrontFlangeBackLength",0.2); // measured STEP


  const std::string mName=mirrorKey+"M3Tube";
  SimpleTubeGen.setCF<CF150>();
  SimpleTubeGen.generateTube(Control,mName,0.0,36.0);
  Control.addVariable(mName+"NPorts",0);
  Control.addVariable(mName+"WallThick",0.25); // measured STEP

  // mirror in M3Tube
  MirrGen.setPlate(28.0,1.0,9.0);  //guess
  // y/z/theta/phi/radius
  MirrGen.generateMirror(Control,mirrorKey+"M3Mirror",
			 -1.0, // x
			 0.0, // y
			 0.0, // z
			 -1.0, // theta
			 0.0, // phi
			 0.0); //radius
  Control.addVariable(mirrorKey+"M3MirrorYAngle",90.0);

  Control.addVariable(mirrorKey+"M3StandHeight",110.0);
  Control.addVariable(mirrorKey+"M3StandWidth",30.0);
  Control.addVariable(mirrorKey+"M3StandLength",30.0);
  Control.addVariable(mirrorKey+"M3StandMat","SiO2");

  PipeGen.setMat("Stainless304");
  PipeGen.setCF<CF63>();
  PipeGen.setAFlangeCF<CF150>();
  PipeGen.generatePipe(Control,mirrorKey+"M3Back",0.0,4.05);
  Control.addVariable(mirrorKey+"M3BackWindowActive",0);
  Control.addVariable(mirrorKey+"M3BackFlangeFrontLength",0.2); // measured STEP

  // M3 STXM part

  PipeGen.setMat("Stainless304");
  PipeGen.setCF<CF100>();
  PipeGen.setBFlangeCF<CF150>();
  PipeGen.generatePipe(Control,mirrorKey+"M3STXMFront",0.0,5.85);
  Control.addVariable(mirrorKey+"M3STXMFrontWindowActive",0);
  Control.addVariable(mirrorKey+"M3STXMFrontFlangeBackLength",0.2);


  const std::string stxmName=mirrorKey+"M3STXMTube";
  SimpleTubeGen.setCF<CF150>();
  SimpleTubeGen.generateTube(Control,stxmName,0.0,36.0);
  Control.addVariable(stxmName+"XStep",0.0);
  Control.addVariable(stxmName+"NPorts",0);

  return;
}

void
opticsSlitPackage(FuncDataBase& Control,
		  const std::string& opticsName)
  /*!
    Builds the DM2 slit package
    \param Control :: Function data base for variables
    \param opticsName :: PreNaem
   */
{
  setVariable::PortTubeGenerator PortTubeGen;
  setVariable::JawValveGenerator JawGen;
  setVariable::PortItemGenerator PItemGen;
  setVariable::BeamPairGenerator BeamMGen;

    /// SLIT PACKAGE

  const std::string sName=opticsName+"SlitTube";
  constexpr double tLen(48.2);
  PortTubeGen.setPipeCF<CF150>();

  PortTubeGen.setPortCF<CF63>();
  PortTubeGen.setPortLength(-5.4,-5.0);
  PortTubeGen.setBPort(7.05, 2.2, 0.5); // R,L,T
  PortTubeGen.setBFlange(10.2, 2.2); // R,L
  PortTubeGen.generateTube(Control,sName,0.0,tLen);
  Control.addVariable(sName+"WallMat", "Aluminium");

  Control.addVariable(sName+"NPorts",3);
  PItemGen.setCF<setVariable::CF100>(7.1);
  PItemGen.setPlate(setVariable::CF100::flangeLength,"Aluminium");

  const Geometry::Vec3D topJaw(0.0,16.0-tLen/2.0,0.0);
  const Geometry::Vec3D sideJaw(0.0,20.0-tLen/2.0,0.0);
  const Geometry::Vec3D vacPort(0.0,0.0,0.0);

  const Geometry::Vec3D XVec(1,0,0);
  const Geometry::Vec3D ZVec(0,0,1);
  constexpr double angle(-45*M_PI/180.0);
  const Geometry::Vec3D port1Vec(cos(angle),sin(angle),0);

  PItemGen.setOuterVoid(1); ///
  PItemGen.generatePort(Control,sName+"Port0",topJaw,ZVec);
  PItemGen.setCF<setVariable::CF50>(6.1);
  PItemGen.setPlate(setVariable::CF40::flangeLength,"SiO2");
  PItemGen.generatePort(Control,sName+"Port1",sideJaw,XVec);
  PItemGen.setCF<setVariable::CF100>(7.1);
  PItemGen.generatePort(Control,sName+"Port2",vacPort,-XVec);

  // Jaw units:
  BeamMGen.setThread(0.5,"Nickel");
  BeamMGen.setLift(0.0,2.5);
  BeamMGen.setGap(1.0,1.0);
  BeamMGen.setXYStep(0.6,0.0,-0.6,0);
  BeamMGen.setBlock(4.0,2.0,1.0,0.0,"Copper");

  const std::string jawKey[]={"JawX","JawZ"};
  for(size_t i=0;i<2;i++)
    {
      const std::string fname=opticsName+jawKey[i];
      BeamMGen.generateMount(Control,fname,1);  // outer of beam
    }

  return;
}


void
opticsVariables(FuncDataBase& Control,
		const std::string& beamName)
  /*
    Vacuum optics components in the optics hutch
    \param Control :: Function data base
    \param beamName :: Name of beamline
  */
{
  ELog::RegMethod RegA("softimaxVariables[F]","opticsVariables");

  const std::string preName(beamName+"OpticsLine");
  const Geometry::Vec3D XVec(1,0,0);
  const Geometry::Vec3D ZVec(0,0,1);
  std::string Name;

  Control.addVariable(preName+"OuterLeft",74.0);
  Control.addVariable(preName+"OuterRight",74.0);
  Control.addVariable(preName+"OuterTop",70.0);

  setVariable::PipeGenerator PipeGen;
  setVariable::BellowGenerator BellowGen;
  setVariable::CrossGenerator CrossGen;
  setVariable::PortTubeGenerator PTubeGen;
  setVariable::PipeTubeGenerator SimpleTubeGen;
  setVariable::PortItemGenerator PItemGen;
  setVariable::GateValveGenerator GateGen;
  setVariable::VacBoxGenerator VBoxGen;
  setVariable::FlangeMountGenerator FlangeGen;
  setVariable::BremCollGenerator BremGen;
  setVariable::BremMonoCollGenerator BremMonoGen;
  setVariable::JawFlangeGenerator JawFlangeGen;
  setVariable::DiffPumpGenerator DiffGen;
  setVariable::JawValveGenerator JawGen;

  PipeGen.setNoWindow();   // no window

  BellowGen.setCF<setVariable::CF40>();
  BellowGen.generateBellow(Control,preName+"InitBellow",0,11.0-1.4);

  // TODO:
  // and set FlangeLength to 1.27 cm (instead of 0.5)

  // will be rotated vertical
  const std::string pipeName=preName+"TriggerPipe";
  SimpleTubeGen.setCF<CF100>();
  SimpleTubeGen.setCap();
  SimpleTubeGen.generateTube(Control,pipeName,0.0,40.0);

  Control.addVariable(pipeName+"NPorts",2);   // beam ports
  // const Geometry::Vec3D ZVec(0,0,1);
  PItemGen.setCF<setVariable::CF40>(5.0);
  PItemGen.setPlate(0.0,"Void");
  PItemGen.generatePort(Control,pipeName+"Port0",Geometry::Vec3D(0,5.0,0),ZVec);
  PItemGen.generatePort(Control,pipeName+"Port1",Geometry::Vec3D(0,5.0,0),-ZVec);

  // will be rotated vertical
  const std::string gateAName=preName+"GateTubeA";
  SimpleTubeGen.setCF<CF63>();
  SimpleTubeGen.setCap();
  SimpleTubeGen.generateTube(Control,gateAName,0.0,30.0);
  Control.addVariable(gateAName+"NPorts",2);   // beam ports

  PItemGen.setCF<setVariable::CF40>(3.45);
  PItemGen.setPlate(0.0,"Void");
  PItemGen.generatePort(Control,gateAName+"Port0",Geometry::Vec3D(0,0,0),ZVec);
  PItemGen.generatePort(Control,gateAName+"Port1",Geometry::Vec3D(0,0,0),-ZVec);

  FlangeGen.setNoPlate();
  FlangeGen.setBlade(4.0,5.0,0.3,0.0,"Stainless304",1);  // 22 rotation
  FlangeGen.generateMount(Control,preName+"GateTubeAItem",0);  // in beam

  BellowGen.setCF<setVariable::CF40>();
  //  BellowGen.setBFlangeCF<setVariable::CF63>();
  BellowGen.generateBellow(Control,preName+"BellowA",0,26.9);

  PipeGen.setMat("Stainless304");
  PipeGen.setCF<CF40>();
  PipeGen.generatePipe(Control,preName+"PipeA",0.0,10.9);
  Control.addVariable(preName+"PipeAWindowActive",0);

  // will be rotated vertical
  const std::string pumpName=preName+"PumpM1";
  SimpleTubeGen.setCF<CF150>();
  SimpleTubeGen.setCap();
  SimpleTubeGen.generateTube(Control,pumpName,0.0,39.2); // full length (+caps)
  Control.addVariable(pumpName+"NPorts",7);

  PItemGen.setCF<setVariable::CF40>(5.3); // port length
  PItemGen.setPlate(0.0,"Void");
  PItemGen.generatePort(Control,pumpName+"Port0",Geometry::Vec3D(0,0,0),ZVec);

  PItemGen.setCF<setVariable::CF63>(4.7);
  PItemGen.setPlate(0.0,"Void");
  PItemGen.setOuterVoid(0);
  PItemGen.generatePort(Control,pumpName+"Port1",Geometry::Vec3D(0,0,0),-ZVec);

  const Geometry::Vec3D pAngVec(0.75,0.0,cos(M_PI*60.0/180.0));
  const double PLen=14.0-8.05/cos(M_PI*37.0/180.0);
  PItemGen.setCF<setVariable::CF40>(PLen);
  PItemGen.setOuterVoid(1);
  PItemGen.setPlate(setVariable::CF40::flangeLength,"Stainless304");
  PItemGen.generatePort(Control,pumpName+"Port2",
			Geometry::Vec3D(0,0,0),-pAngVec);

  PItemGen.setCF<setVariable::CF40>(5.4); // port length
  PItemGen.generatePort(Control,pumpName+"Port3",Geometry::Vec3D(0,0,0),XVec);

  PItemGen.setCF<setVariable::CF40>(5.4); // port length
  PItemGen.generatePort(Control,pumpName+"Port4",Geometry::Vec3D(0,10,0),XVec);

  // above port 2
  PItemGen.setCF<setVariable::CF40>(5.4); // port length
  PItemGen.generatePort(Control,pumpName+"Port5",Geometry::Vec3D(0,10,0),-XVec);

  constexpr double xyAngle6(70.0*M_PI/180.0);
  const Geometry::Vec3D pAngVec6(sin(xyAngle6),0.0,-cos(xyAngle6));
  const double PLen6(14.0-8.05/cos(M_PI*37.0/180.0));
  PItemGen.setCF<setVariable::CF40>(PLen6);
  PItemGen.generatePort(Control,pumpName+"Port6",
			Geometry::Vec3D(0,0,0),-pAngVec6);

  // Gate valve A
  GateGen.setLength(6.3);
  GateGen.setCubeCF<setVariable::CF63>();
  GateGen.generateValve(Control,preName+"GateA",0.0,0);
  Control.addVariable(preName+"GateAPortALen",0.8);

  BellowGen.setCF<setVariable::CF63>();
  //  BellowGen.setBFlangeCF<setVariable::CF150>();
  BellowGen.generateBellow(Control,preName+"BellowB",0,17.6);

  m1MirrorVariables(Control,preName);

  BellowGen.setCF<setVariable::CF63>();
  BellowGen.generateBellow(Control,preName+"BellowC",0,19.7);

  // will be rotated vertical
  const std::string collName=preName+"PumpTubeA";
  SimpleTubeGen.setCF<CF100>(); // counted 16 bolts
  SimpleTubeGen.setCap();
  SimpleTubeGen.generateTube(Control,collName,0.0,30.8);
  Control.addVariable(collName+"NPorts",2);   // beam ports

  PItemGen.setCF<setVariable::CF63>(4.15);
  PItemGen.setPlate(0.0,"Void");
  PItemGen.generatePort(Control,collName+"Port0",Geometry::Vec3D(0,0,0),ZVec);

  PItemGen.setCF<setVariable::CF63>(3.65);
  PItemGen.setPlate(0.0,"Void");
  PItemGen.generatePort(Control,collName+"Port1",Geometry::Vec3D(0,0,0),-ZVec);

  setVariable::BremOpticsCollGenerator OpticsCollGen;
  OpticsCollGen.setCF<setVariable::CF63>();
  OpticsCollGen.generateColl(Control,preName+"BremCollA",0.0, 12.0); // measured: 12 cm total length
  Control.addVariable(preName+"BremCollAExtXStep", 2.3);


  GateGen.setLength(3.5);
  GateGen.setCubeCF<setVariable::CF63>();
  GateGen.generateValve(Control,preName+"GateB",0.0,0);

  BellowGen.setCF<setVariable::CF63>();
  BellowGen.generateBellow(Control,preName+"BellowD",0,21.5);

  opticsSlitPackage(Control,preName);

  monoVariables(Control,preName);

  GateGen.setLength(3.5);
  GateGen.setCubeCF<setVariable::CF63>();
  GateGen.generateValve(Control,preName+"GateC",0.0,0);
  //  Control.addVariable(preName+"GateCXYAngle",1);

  BellowGen.setCF<setVariable::CF63>();
  BellowGen.generateBellow(Control,preName+"BellowE",0,26.5);

  // will be rotated vertical
  const std::string pumpTubeBname=preName+"PumpTubeB";
  SimpleTubeGen.setCF<CF100>(); // counted 16 bolts
  SimpleTubeGen.setCap();
  SimpleTubeGen.generateTube(Control,pumpTubeBname,0.0,31.2);
  Control.addVariable(pumpTubeBname+"NPorts",2);   // beam ports

  PItemGen.setCF<setVariable::CF63>(4.45);
  PItemGen.setPlate(0.0,"Void");
  PItemGen.generatePort(Control,pumpTubeBname+"Port0",Geometry::Vec3D(0,0,0),ZVec);

  PItemGen.setCF<setVariable::CF63>(3.45);
  PItemGen.setPlate(0.0,"Void");
  PItemGen.generatePort(Control,pumpTubeBname+"Port1",Geometry::Vec3D(0,0,0),-ZVec);

  GateGen.setLength(3.5);
  GateGen.setCubeCF<setVariable::CF63>();
  GateGen.generateValve(Control,preName+"GateD",0.0,0);

  PipeGen.setMat("Stainless304");
  PipeGen.setCF<CF40>();
  PipeGen.setAFlangeCF<CF63>();
  PipeGen.setBFlangeCF<CF63>();
  PipeGen.generatePipe(Control,preName+"JoinPipeA",0.0,74.0);

  BellowGen.setCF<setVariable::CF40>();
  BellowGen.setAFlangeCF<setVariable::CF63>();
  BellowGen.setBFlangeCF<setVariable::CF100>();
  BellowGen.generateBellow(Control,preName+"BellowF",0,12.0+2.5);

  JawGen.setRadius(setVariable::CF100::innerRadius);
  JawGen.setWallThick(2.6);
  JawGen.setLength(2.0);
  JawGen.setSlits(3.0,2.0,0.2,"Tantalum"); // W,H,Thick,Mat
  JawGen.generateSlits(Control,preName+"SlitsA",0.0,2.0,3.3);//X,xOpen,zOpen

  /////////////////////////////////// M3 Pump and baffle
  // will be rotated vertical
  const std::string gateName=preName+"PumpTubeM3";
  SimpleTubeGen.setCF<CF150>();
  SimpleTubeGen.setCap();
  SimpleTubeGen.generateTube(Control,gateName,0.0,40.0);
  Control.addVariable(gateName+"NPorts",2);   // beam ports

  PItemGen.setCF<setVariable::CF100>(6.0);
  PItemGen.setPlate(0.0,"Void");
  PItemGen.generatePort(Control,gateName+"Port0",Geometry::Vec3D(0,0,0),ZVec);
  PItemGen.setCF<setVariable::CF63>(6.0);
  PItemGen.setPlate(0.0,"Void");
  PItemGen.generatePort(Control,gateName+"Port1",Geometry::Vec3D(0,0,0),-ZVec);

  FlangeGen.setNoPlate();
  FlangeGen.setBlade(4.0,5.0,0.3,0.0,"Stainless304",1);
  FlangeGen.generateMount(Control,preName+"PumpTubeM3Baffle",0);
  ///////////////////////////////////////////////////////////////////

  BellowGen.setCF<setVariable::CF63>();
  BellowGen.generateBellow(Control,preName+"BellowG",0,17.0);

  m3MirrorVariables(Control,preName);

  BellowGen.setCF<setVariable::CF63>();
  BellowGen.generateBellow(Control,preName+"BellowH",0,16.9);

  GateGen.setCubeCF<setVariable::CF63>();
  GateGen.generateValve(Control,preName+"GateE",0.0,0);

  PipeGen.setMat("Stainless304");
  PipeGen.setCF<CF63>();
  PipeGen.setBFlangeCF<CF100>();
  PipeGen.generatePipe(Control,preName+"JoinPipeB",0.0,7.0);

  //////////// pumpTubeC
    // will be rotated vertical
  const std::string pumpTubeCname=preName+"PumpTubeC";
  SimpleTubeGen.setCF<CF100>(); // counted 16 bolts
  SimpleTubeGen.setCap();
  SimpleTubeGen.generateTube(Control,pumpTubeCname,0.0,31.2);
  Control.addVariable(pumpTubeCname+"NPorts",2);   // beam ports

  PItemGen.setCF<setVariable::CF100>(7.95);
  PItemGen.setPlate(0.0,"Void");
  PItemGen.generatePort(Control,pumpTubeCname+"Port0",Geometry::Vec3D(0,0,0),ZVec);

  PItemGen.setCF<setVariable::CF100>(7.95);
  PItemGen.setPlate(0.0,"Void");
  PItemGen.generatePort(Control,pumpTubeCname+"Port1",Geometry::Vec3D(0,0,0),-ZVec);

  BellowGen.setCF<setVariable::CF100>();
  BellowGen.generateBellow(Control,preName+"BellowI",0,24.5);

  PipeGen.setMat("Stainless304");
  PipeGen.setCF<CF100>();
  PipeGen.generatePipe(Control,preName+"JoinPipeC",0.0,12.5);

  GateGen.setLength(4.5);
  GateGen.setCubeCF<setVariable::CF100>();
  GateGen.generateValve(Control,preName+"GateF",0.0,0);

  BellowGen.setCF<setVariable::CF100>();
  BellowGen.generateBellow(Control,preName+"BellowJ",0,20.0);

  splitterVariables(Control,preName);

  return;
}

void
exptVariables(FuncDataBase& Control,
		      const std::string& beamName)
  /*
    Components in the experimental hutch
    \param Control :: Function data base
    \param beamName :: Name of beamline
  */
{
  const std::string preName(beamName+"ExptLine");

  Control.addVariable(preName+"OuterLength",2300.0);
  Control.addVariable(preName+"OuterLeft",85.0);
  Control.addVariable(preName+"OuterRight",85.0);
  Control.addVariable(preName+"OuterTop",85.0);

  setVariable::BellowGenerator BellowGen;
  setVariable::GateValveGenerator GateGen;
  setVariable::JawValveGenerator JawGen;
  setVariable::PipeGenerator PipeGen;
  setVariable::MonoBoxGenerator VBoxGen;
  setVariable::DiffPumpGenerator DiffGen;
  setVariable::PortItemGenerator PItemGen;

  // Gate valve A - round
  GateGen.setLength(2.5);
  GateGen.setCubeCF<setVariable::CF40>();
  GateGen.generateValve(Control,preName+"GateA",0.0,0);

   // Double slits A and B
  JawGen.setCF<setVariable::CF100>();
  JawGen.setAPortCF<setVariable::CF40>();
  JawGen.setLength(4.0);
  JawGen.setRadius(4.0);
  JawGen.setSlits(3.0,2.0,0.2,"Tantalum");
  JawGen.generateSlits(Control,preName+"DoubleSlitA",0.0,0.8,0.8);

  JawGen.setCF<setVariable::CF100>();
  JawGen.setBPortCF<setVariable::CF40>();
  JawGen.setLength(4.0);
  JawGen.setRadius(4.0);
  JawGen.setSlits(3.0,2.0,0.2,"Tungsten");
  JawGen.generateSlits(Control,preName+"DoubleSlitB",0.0,0.8,0.8);

  VBoxGen.setMat("Stainless304");
  VBoxGen.setWallThick(1.0); // measured
  VBoxGen.setCF<CF63>();
  VBoxGen.setAPortCF<CF40>();
  VBoxGen.setPortLength(2.5,2.5); // La/Lb
  VBoxGen.setLids(3.5,1.5,1.5); // over/base/roof - all values are measured

  const std::string duName(preName+"DiagnosticUnit");

  // arguments: ystep/width/height/depth/length
  VBoxGen.generateBox(Control,duName,
		      0.0,22.0,8.5,8.5,43.0); // measured

  Control.addVariable(duName+"FilterHolder1YStep",8.2);
  Control.addVariable(duName+"FilterHolder1Thick",0.8); // measured
  Control.addVariable(duName+"FilterHolder1Height",1.8);
  Control.addVariable(duName+"FilterHolder1Depth",1.4);
  Control.addVariable(duName+"FilterHolder1Width",5.75);
  Control.addVariable(duName+"FilterHolder1Mat","Stainless304");
  Control.addVariable(duName+"FilterHolder1LegHeight",1.3);
  Control.addVariable(duName+"FilterHolder1LegWidth",1.5);
  Control.addVariable(duName+"FilterHolder1BaseHeight",1.0);
  Control.addVariable(duName+"FilterHolder1BaseWidth",6.5);
  Control.addVariable(duName+"FilterHolder1FoilThick",1.0); // arbitrary
  Control.addVariable(duName+"FilterHolder1FoilMat","Silicon300K"); // arbitrary
  Control.addVariable(duName+"FilterHolder1NWindows",5); // measured
  Control.addVariable(duName+"FilterHolder1WindowHeight",0.6); // measured
  Control.addVariable(duName+"FilterHolder1WindowDepth",0.6); // measured
  Control.addVariable(duName+"FilterHolder1WindowWidth",0.7); // measured

  Control.copyVarSet(duName+"FilterHolder1",duName+"FilterHolder2");
  Control.addVariable(duName+"FilterHolder2YStep",2.0);

  Control.copyVarSet(duName+"FilterHolder1",duName+"FilterHolder3");
  Control.addVariable(duName+"FilterHolder3YStep",2.0);

  // Gate valve B - flat
  GateGen.setLength(2.5);
  GateGen.setCubeCF<setVariable::CF40>();
  GateGen.generateValve(Control,preName+"GateB",0.0,0);

  DiffGen.generatePump(Control,preName+"DiffPump",53.24);
  // NOTE: ACTIVE WINDOW:
  PipeGen.setCF<setVariable::CF40>();
  PipeGen.setWindow(2.7, 0.005);
  PipeGen.setAFlange(2.7,0.5);
  PipeGen.generatePipe(Control,preName+"TelescopicSystem",0,100.0);
  // In reality the window is made of 50 um diamond,
  // but void is a reasonable approximation for our needs:
  // Graphite#2 is Diamond (graphite with double density)
  Control.addVariable(preName+"TelescopicSystemWindowBackMat", "Diamond");
  Control.addVariable(preName+"TelescopicSystemWindowFrontMat", "Diamond");

  // sample area dimensions are arbitrary
  Control.addVariable(preName+"SampleAreaWidth",100.0);
  Control.addVariable(preName+"SampleAreaHeight",50.0);
  Control.addVariable(preName+"SampleAreaDepth",10.0);
  Control.addVariable(preName+"SampleAreaSampleRadius",0.0);  // sample not made
  Control.addVariable(preName+"SampleAreaAirMat","Air");
  Control.addVariable(preName+"SampleAreaSampleMat","Stainless304");

  const std::string tubeName(preName+"Tube");

  // X032_SoftiMAX_\(2019-02-11\)_dimensions.pdf:
  Control.addVariable(tubeName+"YStep", 454.748); // dummy

  const std::string noseName(tubeName+"NoseCone");

  Control.addVariable(noseName+"Length",35.0); // measured
  Control.addVariable(noseName+"MainMat","Void"); //
  Control.addVariable(noseName+"WallMat","Stainless304");
  Control.addVariable(noseName+"WallThick",1.0); // measured

  Control.addVariable(noseName+"FrontPlateWidth",12.0); // measured
  Control.addVariable(noseName+"FrontPlateHeight",12.0); // measured
  Control.addVariable(noseName+"FrontPlateThick",1.5); // measured

  Control.addVariable(noseName+"BackPlateWidth",38.0); // measured
  Control.addVariable(noseName+"BackPlateHeight",38.0); // measured
  Control.addVariable(noseName+"BackPlateThick",2.5); // measured
  Control.addVariable(noseName+"BackPlateRimThick",4.5); // measured

  Control.addVariable(noseName+"PipeRadius",4.0); // ??? guess
  Control.addVariable(noseName+"PipeLength",4.6); // measured
  Control.addVariable(noseName+"PipeWallThick",
		      static_cast<double>(setVariable::CF63::wallThick)); // guess ???
  Control.addVariable(noseName+"FlangeRadius",
		      static_cast<double>(setVariable::CF63::flangeRadius));
  Control.addVariable(noseName+"FlangeLength",2.6); // measured

  // front window
  Control.addVariable(noseName+"WindowRadius",setVariable::CF63::wallThick/2.0);
  Control.addVariable(noseName+"WindowThick",0.05);
  Control.addVariable(noseName+"WindowMat","Graphite");

  GateGen.setLength(10.0);
  GateGen.setCubeCF<setVariable::CF40>();
  GateGen.generateValve(Control,tubeName+"GateA",0.0,0);
  Control.addVariable(tubeName+"GateARadius",17.0); // measured

  // [1] = x032_softimax_-2019-02-11-_dimensions.pdf
  // [2] = measured in X032_SoftiMAX_(2019-04-25).step
  Control.addVariable(tubeName+"StartPlateThick", 2.7); // [1]
  Control.addVariable(tubeName+"StartPlateRadius", 57.8);  // [1], 1156/2.0 mm

  // According to [1], the PortRadius is 50.2/2 = 25.1 cm, but here we set it to
  // 14.27 cm - the port radius of the gasket plate betwen GateA and StartPlate
  // (which we do not build)
  Control.addVariable(tubeName+"StartPlatePortRadius", 14.27);
  Control.addVariable(tubeName+"StartPlateMat", "Stainless304");


  const Geometry::Vec3D C(0,0,0);
  const Geometry::Vec3D C1(0,0.1,0);
  const Geometry::Vec3D C2(0,55.1,0);
  const Geometry::Vec3D C3(0,-50.7,0);

  const Geometry::Vec3D PX(1,0,0);
  const Geometry::Vec3D PY(0,1,0);
  const Geometry::Vec3D PZ(0,0,1);

  setVariable::PipeTubeGenerator SimpleTubeGen;
  SimpleTubeGen.setPipe(50.2,0.6,57.8,4.3);  // Rad,thick,Flange (Rad,len)

  std::string segName=tubeName+"Segment1";
  SimpleTubeGen.generateTube(Control,segName,0.0,167.2);
  Control.addVariable(segName+"NPorts",1);

  PItemGen.setCF<setVariable::CF350>(7.0);
  PItemGen.setPlate(CF350::flangeLength,"Stainless304");
  PItemGen.setOuterVoid(1);
  PItemGen.generatePort(Control,segName+"Port0",C1,PX);

  // segment 2:
  segName=tubeName+"Segment2";
  SimpleTubeGen.generateTube(Control,segName,0.0,176);
  Control.addVariable(segName+"NPorts",1);
  PItemGen.generatePort(Control,segName+"Port0",C,-PX);

  // segment 3: short without ports before the wall
  segName=tubeName+"Segment3";
  setVariable::PipeTubeGenerator WallTubeGen(SimpleTubeGen);
  WallTubeGen.setAFlange(57.8,3.7);
  WallTubeGen.setBFlange(70.0,1.0);
  // [2] 1 added to have distance 378.7 as in [1]
  WallTubeGen.generateTube(Control,segName,0.0,32.8+1.0);
  Control.addVariable(segName+"NPorts",0);


  // segment 4: longer with 2 ports right after the wall
  segName=tubeName+"Segment4";
  SimpleTubeGen.setAFlange(70.0,1.0);
  SimpleTubeGen.generateTube(Control,segName,0.0,238.2);

  Control.addVariable(segName+"NPorts",2);
  PItemGen.generatePort(Control,segName+"Port0",Geometry::Vec3D(0,38.2,0),PX);
  PItemGen.generatePort(Control,segName+"Port1",Geometry::Vec3D(0,-67.6,0),-PX);
  // segments 5-9 are the same length [5 has more ports]
  setVariable::PortItemGenerator PItemExtraGen(PItemGen);
  PItemExtraGen.setPort(19.0,17.8,0.6);          // len/rad/wall
  PItemExtraGen.setFlange(20.0,1.0);
  PItemExtraGen.setPlate(2.5,"Stainless304");

  // Segment 5
  segName=tubeName+"Segment5";
  SimpleTubeGen.setAFlange(57.8,4.3);   // set back to default
  SimpleTubeGen.generateTube(Control,segName,0.0,264.0);
  Control.addVariable(segName+"NPorts",5);

  constexpr double alpha(30*M_PI/180);
  PItemGen.generatePort(Control,segName+"Port0",C2,PX);
  PItemGen.generatePort(Control,segName+"Port1",C3,-PX);
  PItemExtraGen.generatePort(Control,segName+"Port2",
			     Geometry::Vec3D(0,3.3,0),
			     Geometry::Vec3D(0,-sin(alpha),-cos(alpha)));
  PItemExtraGen.generatePort(Control,segName+"Port3",
			     Geometry::Vec3D(0,60.9,0),
			     Geometry::Vec3D(0,-sin(alpha),-cos(alpha)));

  PItemExtraGen.setPort(7.0,10.0,0.6);
  PItemExtraGen.setFlange(12.0,2.5);
  PItemExtraGen.generatePort(Control,segName+"Port4",
			     Geometry::Vec3D(0,-20.0,0),PX);

  // segments 6
  segName=tubeName+"Segment6";
  SimpleTubeGen.generateTube(Control,segName,0.0,264.0);
  Control.addVariable(segName+"NPorts",2);
  PItemGen.generatePort(Control,segName+"Port0",C2,PX);
  PItemGen.generatePort(Control,segName+"Port1",C3,-PX);

  // segments 7
  segName=tubeName+"Segment7";
  SimpleTubeGen.generateTube(Control,segName,0.0,264.0);
  Control.addVariable(segName+"NPorts",2);
  PItemGen.generatePort(Control,segName+"Port0",C2,PX);
  PItemGen.generatePort(Control,segName+"Port1",C3,-PX);

  // segments 8
  segName=tubeName+"Segment8";
  SimpleTubeGen.setAFlange(57.8,4.0);
  SimpleTubeGen.setBFlange(57.8,4.0);
  SimpleTubeGen.setFlangeCap(0.0,2.7);

  SimpleTubeGen.generateTube(Control,segName,0.0,264);
  //  SimpleTubeGen.setFlange(4.)
  Control.addVariable(segName+"NPorts",4);
  PItemGen.generatePort(Control,segName+"Port0",C2,PX);
  PItemGen.generatePort(Control,segName+"Port1",C3,-PX);

  PItemGen.setPort(6.6,4,1.0);  // len/rad/wall
  PItemGen.setFlange(8.3,2.0);  // rad/len
  PItemGen.setPlate(0.7,"Stainless304");
  PItemGen.generatePort(Control,segName+"Port2",
			Geometry::Vec3D(34.8,0.0,0),PY);
  PItemGen.generatePort(Control,segName+"Port3",
			Geometry::Vec3D(-34.8,0.0,0),PY);


  Control.addParse<double>(tubeName+"OuterRadius",
			   tubeName+"Segment3FlangeBRadius+10.0");
  Control.addParse<double>(tubeName+"OuterLength",
			   "SoftiMAXExptLineTubeNoseConeLength+"
			   "SoftiMAXExptLineTubeSegment1Length+"
			   "SoftiMAXExptLineTubeSegment2Length+"
			   "SoftiMAXExptLineTubeSegment3Length+"
			   "SoftiMAXExptLineTubeSegment4Length+"
			   "SoftiMAXExptLineTubeSegment5Length+"
			   "SoftiMAXExptLineTubeSegment6Length+"
			   "SoftiMAXExptLineTubeSegment7Length+"
			   "SoftiMAXExptLineTubeSegment8Length+"
			   "100.0");



  Control.addVariable(tubeName+"CableWidth",  20.0); // [2]
  Control.addVariable(tubeName+"CableHeight", 10.0); // [2]
  Control.addVariable(tubeName+"CableZStep",  -21.7); // [2]
  Control.addVariable(tubeName+"CableLength", 870.0); // dummy
  Control.addVariable(tubeName+"CableTailRadius", 17.0);
  Control.addVariable(tubeName+"CableMat", "StbTCABL"); // some generic cable material

  Control.addVariable(tubeName+"DetYStep", 0.0);

  Control.addVariable(tubeName+"BeamDumpLength", 0.6); // [2]
  Control.addVariable(tubeName+"BeamDumpRadius", 0.15); // [2]
  Control.addVariable(tubeName+"BeamDumpMat", "Tantalum");

  Control.addVariable(tubeName+"WAXSLength", 34.5); // [2]
  Control.addVariable(tubeName+"WAXSWidth", 23.52); // [2]
  Control.addVariable(tubeName+"WAXSHeight", 22.3); // [2]
  Control.addVariable(tubeName+"WAXSWallThick", 0.3); // [2]
  Control.addVariable(tubeName+"WAXSMainMat", "StbTCABL"); // guess
  Control.addVariable(tubeName+"WAXSWallMat", "Aluminium");
  Control.addVariable(tubeName+"WAXSYStep", 0.0);

  Control.addVariable(tubeName+"AirBoxLength", 32.0); // [2]
  Control.addVariable(tubeName+"AirBoxWidth", 30.8); // [2]
  Control.addVariable(tubeName+"AirBoxHeight", 53.0); // [2]
  Control.addVariable(tubeName+"AirBoxWallThick", 0.3); // [2]
  Control.addVariable(tubeName+"AirBoxMainMat", "Air");
  Control.addVariable(tubeName+"AirBoxWallMat", "Aluminium");

  Control.addVariable(tubeName+"CableWidth",  20.0); // [2]
  Control.addVariable(tubeName+"CableHeight", 10.0); // [2]
  Control.addVariable(tubeName+"CableZStep",  0.1);//-21.7); // [2]
  Control.addVariable(tubeName+"CableLength", 750.0); // dummy
  Control.addVariable(tubeName+"CableTailRadius", 17.0);
  Control.addParse<double>(tubeName+"CableDetYStep", tubeName+"WAXSYStep");
  Control.addVariable(tubeName+"CableMat", "StbTCABL"); // some generic cable material

  return;
}

}  // NAMESPACE softimaxVAR

void
SOFTIMAXvariables(FuncDataBase& Control)
  /*!
    Function to set the control variables and constants
    -- This version is for Photon Moderator
    \param Control :: Function data base to add constants too
  */
{
  ELog::RegMethod RegA("softimaxVariables[F]","softimaxVariables");

  Control.addVariable("sdefType","Wiggler");

  setVariable::PipeGenerator PipeGen;
  setVariable::LeadPipeGenerator LeadPipeGen;
  setVariable::PipeShieldGenerator ShieldGen;

  PipeGen.setWindow(-2.0,0.0);   // no window

  softimaxVar::undulatorVariables(Control,"SoftiMAXFrontBeam");

  /// Parameters of R3FrontEndVariables:
  // 25 =exitLeng :: last exit pipe length
  setVariable::R3FrontEndVariables(Control,"SoftiMAXFrontBeam",25.0);
  softimaxVar::frontMaskVariables(Control,"SoftiMAXFrontBeam");

  softimaxVar::wallVariables(Control,"SoftiMAXWallLead");

  PipeGen.setMat("Stainless304");
  PipeGen.setCF<setVariable::CF40>(); // CF40 was 2cm (why?)
  PipeGen.setBFlange(3.5,0.3);
  PipeGen.generatePipe(Control,"SoftiMAXJoinPipe",0,158.95); // length adjusted to place M1 at 2400 from undulator centre

  softimaxVar::opticsHutVariables(Control,"SoftiMAX");
  Control.addVariable("SoftiMAXOpticsHutVoidMat", "Void");

  softimaxVar::opticsVariables(Control,"SoftiMAX");
  //  softimaxVar::exptHutVariables(Control,"SoftiMAX");
  //  softimaxVar::exptVariables(Control,"SoftiMAX");

  PipeGen.generatePipe(Control,"SoftiMAXJoinPipeB",0,100.0);

  ShieldGen.setPlate(60.0,60.0,10.0);
  ShieldGen.generateShield(Control,"SoftiMAXScreenA",4.4,0.0);

  return;
}

}  // NAMESPACE setVariable
