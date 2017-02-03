/********************************************************************* 
  CombLayer : MCNP(X) Input builder
 
 * File:   commonVar/CryoGenerator.cxx
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
#include <stack>
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
#include "stringCombine.h"
#include "MatrixBase.h"
#include "Matrix.h"
#include "Vec3D.h"
#include "varList.h"
#include "Code.h"
#include "FuncDataBase.h"
#include "CryoGenerator.h"

namespace setVariable
{

CryoGenerator::CryoGenerator() 
  /*!
    Constructor : All variables are set for 35cm radius disks
    with an overlap of 5cm. Values are scaled appropiately for
    most changes
  */
{}


CryoGenerator::~CryoGenerator() 
 /*!
   Destructor
 */
{}

  
void
CryoGenerator::generateFridge(FuncDataBase& Control,
                               const std::string& keyName,
                               const double yStep)
  /*!
    Generate the chopper variables
    \param Control :: Functional data base
    \param keyName :: Base name for chopper variables
    \param yStep :: main y-step
   */
{
  ELog::RegMethod RegA("CryoGenerator","generateCryostate");

  Control.addVariable(keyName+"YStep",yStep);


  Control.addVariable(keyName+"SampleRadius",0.75);
  Control.addVariable(keyName+"SampleHeight",2.0);
  
  Control.addVariable(keyName+"VoidRadius",4.5);
  Control.addVariable(keyName+"VoidDepth",7.0);
  Control.addVariable(keyName+"VoidHeight",3.0);
  Control.addVariable(keyName+"VoidWallThick",0.5);

  Control.addVariable(keyName+"StickLen",190.0);
  Control.addVariable(keyName+"StickRadius",0.6);

  Control.addVariable(keyName+"StickBoreRadius",1.8);
  Control.addVariable(keyName+"StickBoreHeight",180.0);
  Control.addVariable(keyName+"StickBoreThick",0.5);

  Control.addVariable(keyName+"HeatRadius",6.0);
  Control.addVariable(keyName+"HeatHeight",6.0);
  Control.addVariable(keyName+"HeatDepth",11.0);
  Control.addVariable(keyName+"HeatThick",0.5);


  Control.addVariable(keyName+"TailRadius",8.0);
  Control.addVariable(keyName+"TailHeight",5.0);
  Control.addVariable(keyName+"TailDepth",12.5);
  Control.addVariable(keyName+"TailThick",0.5);

  Control.addVariable(keyName+"TailOuterRadius",18.0);
  Control.addVariable(keyName+"TailOuterHeight",150.0);
  Control.addVariable(keyName+"TailOuterLift",9);

  Control.addVariable(keyName+"SampleMat","H2O");
  Control.addVariable(keyName+"WallMat","Aluminium");
  Control.addVariable(keyName+"StickMat","Stainless304");


  return;
}


  
}  // NAMESPACE setVariable
