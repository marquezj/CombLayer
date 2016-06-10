/********************************************************************* 
  CombLayer : MNCPX Input builder
 
 * File:   t1UpgradeInc/CH4PreMod.h
*
 * Copyright (c) 2004-2013 by Stuart Ansell
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
#ifndef ts1System_CH4PreMod_h
#define ts1System_CH4PreMod_h

class Simulation;

namespace ts1System
{

/*!
  \class CH4PreMod
  \version 1.0
  \author S. Ansell
  \date November 2012
  \brief Pre-mod for H2 (wrap around)
*/

class CH4PreMod : public CH4PreModBase
{
 private:
  
  double sideThick;             ///< Side of water thickness
  double topThick;              ///< Total height
  double baseThick;             ///< top extra
  double frontExt;              ///< +ve beyond moderator
  double backExt;               ///< +ve beyond moderator
  
  size_t touchSurf;             ///< Touching surfae [if exist]

  double alThick;        ///< Al wall thickness
  double vacThick;       ///< Vac wall thickness

  double modTemp;           ///< Moderator material
  int modMat;               ///< Moderator material
  int alMat;                ///< Al material

  /// Points to connect to [inner]
  std::vector<Geometry::Vec3D> sidePts;
  /// Axis of connection [inner=>outer]
  std::vector<Geometry::Vec3D> sideAxis;
  
  void getConnectPoints(const attachSystem::FixedComp&,
			const size_t);

  void populate(const Simulation&);
  void createUnitVector(const attachSystem::FixedComp&);
  void createSurfaces(const attachSystem::FixedComp&,const size_t);
  void createObjects(Simulation&,const attachSystem::FixedComp&,
		     const size_t);
  void createLinks();
  
 public:

  CH4PreMod(const std::string&);
  CH4PreMod(const CH4PreMod&);
  CH4PreMod& operator=(const CH4PreMod&);
  virtual ~CH4PreMod();
  virtual CH4PreMod* clone() const;

  virtual size_t getNLayers(const size_t) const;
  virtual Geometry::Vec3D 
    getSurfacePoint(const size_t,const long int) const;
  virtual int 
    getLayerSurf(const size_t,const size_t) const;
  virtual std::string 
    getLayerString(const size_t,const size_t) const;

  void createAll(Simulation&,const attachSystem::FixedComp&,
		 const size_t,const size_t);

};

}

#endif
 
