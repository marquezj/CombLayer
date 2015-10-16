/********************************************************************* 
  CombLayer : MCNP(X) Input builder
 
 * File:   essBuildInc/F5Calc.cxx
 *
 * Copyright (c) 2015 Konstantin Batkov
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
#include "MatrixBase.h"
#include "Matrix.h"
#include "Vec3D.h"

#include "F5Calc.h"


namespace tallySystem
{

void
F5Calc::SetTally(double lx,double ly,double lz)
{
  F5(lx,ly,lz);
  return;
}
  
void
F5Calc::SetPoints(const Geometry::Vec3D& lB,
		  const Geometry::Vec3D& lC,
		  const Geometry::Vec3D& lB2)
{
  B = lB;
  C = lC;
  B2 = lB2;
  return;
}

void
F5Calc::SetLength(const double laf)
{
  CalculateMiddleOfModerator();
  af=laf;
  
  const double mf=F5.Distance(M);
  A=F5+(M-F5)*(af/mf);
  // // check if A belongs to MF5:
  // double deltaXY = (A.x-M.x)*(F5.y-M.y) - (A.y-M.y)*(F5.x-M.x);
  // double deltaXZ = (A.x-M.x)*(F5.z-M.z) - (A.z-M.z)*(F5.x-M.x);
  // double epsilon = 1E-5;
  return;
}

void
F5Calc::CalculateMiddleOfModerator()
{
  M=C+((B-C)+(B2-B))/2.0;
  return;
}
  
double
F5Calc::GetZAngle()
  
  /*!
    The angle is calculated between the plane, which is normal 
    to the moderator surface and includes point M, and the line M->F5
    The angle is negative when the F5 tally is below the plane
  */
{

  Geometry::Vec3D lF5_Projected;
  //Normalvector of the plane containing B,C and B2
  Geometry::Vec3D lNV; 
  Geometry::Vec3D lNV2; 
  Geometry::Vec3D lM;

  lM=M+(B-C)/2.0;

  lNV=CalculateNormalVector(B,C,B2);

  lNV2=lNV*(lM-M);
  lNV2.makeUnit();

  CalculateProjectionOntoPlane(lNV2,M,F5,lF5_Projected); 
  
  const double lAngle=(180.0/M_PI)*
    fabs(atan(F5.Distance(lF5_Projected))/
	 M.Distance(lF5_Projected));

  // This is junk: Should be relative to the lF5-projection anlge
  return (lF5_Projected.Z()-F5.Z() >= 0.0) ? -lAngle : lAngle;

}
  
double F5Calc::GetXYAngle()

  /*!
    The angle is calculated between
    1. the line consisting of
    a. M (middle of the moderator)
    b. lF5_Projected (the projection of the F5 tally onto the plane which is 
    perpendicular to the moderator surface and includes M)
    2. the line which is perpendicular to the moderator surface and 
    goes through M (middle of the moderator)
  */
{
  return 180.0+atan2(F5.Y()-M.Y(), F5.X()-M.X())*180/M_PI;
}

double
F5Calc::GetHalfSizeX()
  /*!
    This is a horrible way to to this -- use standard pythgerous.
   */
{
  double val = B.Distance(C)/2.0*af/F5.Distance(M);

  const double ab = (F5.X()-M.X())*(B.X()-C.X()) +
    (F5.Y()-M.Y())*(B.Y()-C.Y());

  const double len =
    sqrt(pow(F5.X()-M.X(),2)+pow(F5.Y()-M.Y(),2)) *
    sqrt(pow(B.X()-C.X(),2)+pow(B.Y()-C.Y(),2));
  
  const double cosa = ab/len;
  val *= cos(M_PI/2.0-acos(cosa));
  return val;
};
  
double
F5Calc::GetHalfSizeZ()
{
  return B.Distance(B2)/2.0*af/F5.distance(M);
}

void
F5Calc::CalculateNormalVector(const Geometry::Vec3D& O,
			      const Geometry::Vec3D& P1,
			      const Geometry::Vec3D& P2) const ;
   // STATIC
{
  // bizare way to do cross product normals
  const Geometry::Vec3D pA=O-P3;
  const Geometry::Vec3D pB=O-P2;
  return (pA*pB).makeUnit();
  
  // P4.x=(P1.y-P3.y)*(P1.z-P2.z)-(P1.z-P3.z)*(P1.y-P2.y);
  // P4.y=(P1.z-P3.z)*(P1.x-P2.x)-(P1.x-P3.x)*(P1.z-P2.z);
  // P4.z=(P1.x-P3.x)*(P1.y-P2.y)-(P1.y-P3.y)*(P1.x-P2.x);

  // P4.makeUnit();
  return;
}


void
F5Calc::CalculateProjectionOntoPlane
(Geometry::Vec3D NV,Geometry::Vec3D M,Geometry::Vec3D P,Geometry::Vec3D &P2)
  {
    //NV is the normal vector of the plane
    //M is a point on the plane
    //P is a point which projection (P2) is calculated on the plane

    const double lDistance=
      fabs(NV.x*(M.x-P.x)+NV.y*(M.y-P.y)+NV.z*(M.z-P.z))/sqrt(pow(NV.x,2.0)+pow(NV.y,2.0)+pow(NV.z,2.0));

    //Calculate the possible projections
    Geometry::Vec3D lProjection1,lProjection2;
    lProjection1=P+NV*lDistance;
    lProjection2=P+NV*lDistance;

    //The smaller distance is the correct projection
    if(CalculateDistance(M,lProjection1)<=CalculateDistance(M,lProjection2))
      {
	P2.x=lProjection1.x;
	P2.y=lProjection1.y;
	P2.z=lProjection1.z;
      }
    else
      {
	P2.x=lProjection2.x;
	P2.y=lProjection2.y;
	P2.z=lProjection2.z;
      }
    return;
  }

}
