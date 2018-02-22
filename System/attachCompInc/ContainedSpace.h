/********************************************************************* 
  CombLayer : MCNP(X) Input builder
 
 * File:   attachCompInc/ContainedSpace.h
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
#ifndef attachSystem_ContainedSpace_h
#define attachSystem_ContainedSpace_h

class Simulation;

namespace Geometry
{
  class Line;
}

namespace attachSystem
{
/*!
  \class ContainedSpace
  \version 1.0
  \author S. Ansell
  \date Febrary 2018
  \brief Allows an object to build a divided section
         within the Contained space
*/

class ContainedSpace  : public ContainedComp
{
 private:

  size_t nDirection;                   ///< No of direction of cut
  int primaryCell;                     ///< Master cell
  int cellIndex;                       ///< Space for new cell(s)
  HeadRule BBox;                       ///< Bounding box
  std::vector<LinkUnit> LCutters;      ///< Cutting dividers
  
 public:

  ContainedSpace();
  ContainedSpace(const ContainedSpace&);
  ContainedSpace& operator=(const ContainedSpace&);
  virtual ~ContainedSpace();

  void setPrimaryCell(const int);
  void setConnect(const size_t,const Geometry::Vec3D&,
		      const Geometry::Vec3D&);
  void setLinkSurf(const size_t,const int);
  void setLinkSurf(const size_t,const HeadRule&);
  void setLinkCopy(const size_t,const FixedComp&,const long int);

  /// Get bounding box
  const HeadRule& getBBox() const { return BBox; }
  
  void calcBoundary(Simulation&,const int,const size_t =6);
  void insertObjects(Simulation&);


  
};

}

#endif
 
