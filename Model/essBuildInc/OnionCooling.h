#ifndef essSystem_OnionCooling_h
#define essSystem_OnionCooling_h

class Simulation;

namespace essSystem
{

/*!
  \class OnionCooling
  \date March 2014
  \brief Onion cooling pipes
*/

class OnionCooling :
  public attachSystem::ContainedComp,
    public attachSystem::FixedOffset
{
 private:

  double wallThick;                   ///< Wall thickness
  int wallMat;                        ///< material
  double wallTemp;                    ///< wall temperature

  size_t nRings;                      ///< number of rings
  std::vector<double> radius;         ///< Radius of the rings
  std::vector<double> gateWidth;      ///< full width of spacing in ring
  std::vector<double> gateLength;     ///< length of "the door" in  ring

  // Functions:

  void populate(const FuncDataBase&);

  void createSurfaces();
  void createObjects(Simulation&, const attachSystem::FixedComp&);
  void createLinks();

 public:

  OnionCooling(const std::string&);
  OnionCooling(const OnionCooling&);
  OnionCooling& operator=(const OnionCooling&);
  virtual OnionCooling* clone() const;
  virtual ~OnionCooling();

  void setBottomSurface(const attachSystem::FixedComp& FC, const long int link);
  void setUpperSurface(const attachSystem::FixedComp& FC, const long int link);

  int getMainCell() const { return buildIndex+1; }
  virtual void addToInsertChain(attachSystem::ContainedComp&) const;

  void createAll(Simulation&,const attachSystem::FixedComp&,const long int); 
  
};

}

#endif
 
