/********************************************************************* 
  CombLayer : MCNP(X) Input builder
 
 * File:   weightsInc/FCLConstructor.h
 *
 * Copyright (c) 2004-2016 by Stuart Ansell
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
#ifndef WeightSystem_FCLConstructor_h
#define WeightSystem_FCLConstructor_h

///\file 

class Simulation;
namespace Geometry
{
  class Plane;
}

/*!
  \namespace WeightSystem
  \brief Adds a layer of shutters to the Target/Reflect/Moderatr
  \author S. Ansell
  \version 1.0
  \date April 2009
*/

class Simluation;

namespace mainSystem
{
  class inputParam;
}

namespace physicsSystem
{
  
class FCLConstructor 
{
 private:

  void writeHelp(std::ostream&) const;
    
 public:

  FCLConstructor() {}
  FCLConstructor(const FCLConstructor&) {}
  FCLConstructor& operator=(const FCLConstructor&) { return *this; }
  ~FCLConstructor() {}  ///< Destructor

  void init(Simulation&);
  void processUnit(Simulation&,const mainSystem::inputParam&,
		   const size_t);
};

}

#endif
 
