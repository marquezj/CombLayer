/********************************************************************* 
  CombLayer : MCNP(X) Input builder
 
 * File:   tallyInc/meshConstruct.h
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
#ifndef tallySystem_meshConstruct_h
#define tallySystem_meshConstruct_h

namespace attachSystem
{
  class FixedComp;
}

namespace mainSystem
{
  class inputParam;
}

class objectGroups;
class Simulation;

namespace tallySystem
{

/*!
  \class meshConstruct
  \version 1.0
  \author S. Ansell
  \date July 2012
  \brief Constructs a mesh tally from inputParam
*/

class meshConstruct 
{
 protected:
  
  /// Private constructor
  meshConstruct() {}

  static const std::string& getDoseConversion();
  static const std::string& getPhotonDoseConversion();
  static void calcXYZ(const objectGroups&,
		      const std::string&,const std::string&,
		      Geometry::Vec3D&,Geometry::Vec3D&) ;



 public:

  static void getObjectMesh(const objectGroups&,
			    const mainSystem::inputParam&,
			    const std::string&,
			    const size_t,const size_t,
			    Geometry::Vec3D&,
			    Geometry::Vec3D&,
			    std::array<size_t,3>&);

  static void getFreeMesh(const mainSystem::inputParam&,
			  const std::string&,
			  const size_t,const size_t,
			  Geometry::Vec3D&,
			  Geometry::Vec3D&,
			  std::array<size_t,3>&);

  
  static void writeHelp(std::ostream&);
};

}

#endif
 
