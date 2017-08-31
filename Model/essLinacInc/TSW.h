/********************************************************************* 
  CombLayer : MCNP(X) Input builder
 
 * File:   essBuildInc/TSW.h
 *
 * Copyright (c) 2017 by Konstantin Batkov
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
#ifndef essSystem_TSW_h
#define essSystem_TSW_h

class Simulation;

namespace essSystem
{

/*!
  \class TSW
  \version 1.0
  \author Konstantin Batkov
  \date 31 Aug 2017
  \brief Temporary shielding wall
*/

class TSW : public attachSystem::ContainedComp,
  public attachSystem::FixedOffset
{
 private:

  const std::string baseName;   ///< base name
  const int surfIndex;             ///< Index of surface offset
  int cellIndex;                ///< Cell index

  double length;                ///< Total length including void
  double width;                 ///< Width
  size_t nLayers;               ///< Number of layers for variance reduction

  int mat;                      ///< material
  
  void populate(const FuncDataBase&);
  void createUnitVector(const attachSystem::FixedComp&,
			const long int);
  
  void createSurfaces(const attachSystem::FixedComp&,const long int);
  void createObjects(Simulation&,const attachSystem::FixedComp&,
		     const long int,const long int,const long int,const long int);
  void createLinks(const attachSystem::FixedComp&,
		   const long int,const long int,
		   const long int,const long int);
  
 public:

  TSW(const std::string&,const std::string&,const size_t&);
  TSW(const TSW&);
  TSW& operator=(const TSW&);
  virtual TSW* clone() const;
  virtual ~TSW();
  
  void createAll(Simulation&,const attachSystem::FixedComp&,
		 const long int,const long int,
		 const long int,const long int);

};

}

#endif
 
