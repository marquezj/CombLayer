/********************************************************************* 
  CombLayer : MCNP(X) Input builder
 
 * File:   physicsInc/flukaPhysics.h
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
#ifndef flukaSystem_flukaPhysics_h
#define flukaSystem_flukaPhysics_h

namespace flukaSystem
{
  
/*!
  \class flukaPhysics
  \version 1.0
  \date March 2018
  \author S.Ansell
  \brief Processes the physics cards in the FLUKA output
*/

class flukaPhysics 
{
 private:

  std::vector<int> cellN;            ///< Cell numbers in order
  std::map<int,double> allImp;       ///< all particle
  std::map<int,double> hadronImp;    ///< hadron
  std::map<int,double> electronImp;  ///< electron
  std::map<int,double> lowImp;       ///< lowImp

  bool cellSplit(const std::map<int,double>&,
		 std::vector<std::tuple<int,int,double>>&) const;

 public:
   
  flukaPhysics();
  flukaPhysics(const flukaPhysics&);
  flukaPhysics& operator=(const flukaPhysics&);
  virtual ~flukaPhysics();

  void clearAll();
  
  // ALL systems setup
  void setCellNumbers(const std::vector<int>&);


  void setImp(const std::string&,const int,const double);
  void writeFLUKA(std::ostream&) const;
};

}

#endif
