/********************************************************************* 
  CombLayer : MCNP(X) Input builder
 
 * File:   bibBuildInc/ConcreteWall.h
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
#ifndef bibSystem_concrete_wall_h
#define bibSystem_concrete_wall_h

class Simulation;


namespace bibSystem
{

/*!
  \class ConcreteWall
  \version 1.0
  \author Bilbao team
  \date May 2013
  \brief Concrete Wall around monolyth
*/

class ConcreteWall : public attachSystem::ContainedComp,
    public attachSystem::FixedOffset
{
 private:
  

  double innerRadius;          ///< Main depth
  double thickness;            ///< Main thickness    
  double height;               ///< Height of wall 
  double base;                 ///< Base depth

  int mat;             ///< Default material

  void populate(const FuncDataBase&);
  void createUnitVector(const attachSystem::FixedComp&, const long int );
  void createSurfaces(const attachSystem::FixedComp&, const long int );
  void createLinks();
  void createObjects(Simulation&);

 public:

  ConcreteWall(const std::string&);
  ConcreteWall(const ConcreteWall&);
  ConcreteWall& operator=(const ConcreteWall&);
  virtual ~ConcreteWall();

  std::string getComposite(const std::string&) const;
  void addToInsertChain(attachSystem::ContainedComp& CC) const;
  void addToInsertControl(Simulation&,
			  const attachSystem::FixedComp&,
			  attachSystem::ContainedComp& CC) const;
  /// Main cell
  int getInnerCell() const { return buildIndex+1; }
  std::vector<int> getCells() const;
  void createAll(Simulation&,const attachSystem::FixedComp&,const size_t,
		  const attachSystem::FixedComp&,
	       const size_t );

};

}

#endif
 
