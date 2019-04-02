/********************************************************************* 
  CombLayer : MCNP(X) Input builder
 
 * File:   flukaMagnetInc/magnetDipole.h
 *
 * Copyright (c) 2004-2019 by Stuart Ansell
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
#ifndef flukaSystem_magnetDipole_h
#define flukaSystem_magnetDipole_h

namespace flukaSystem
{
  
/*!
  \class magnetDipole
  \version 1.0
  \date February 2019
  \author S.Ansell
  \brief Holds an external magnetic system
*/

class magnetDipole :
  public magnetUnit
{
 private:

  double KFactor;        ///< K factor [T/cm]
  double Radius;         ///< Radius of the bend [cm]

  virtual void populate(const FuncDataBase&);
  
 public:

  magnetDipole(const std::string&,const size_t);
  magnetDipole(const magnetDipole&);
  magnetDipole& operator=(const magnetDipole&);
  virtual ~magnetDipole();

  void createAll(Simulation&,const attachSystem::FixedComp&,
		 const long int);
  void createAll(Simulation&,const Geometry::Vec3D&,
		 const Geometry::Vec3D&,const Geometry::Vec3D&,
		 const Geometry::Vec3D&,const double,const double);

  virtual void writeFLUKA(std::ostream&) const;

};

}

#endif
