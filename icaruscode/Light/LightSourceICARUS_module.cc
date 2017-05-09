////////////////////////////////////////////////////////////////////////
// Class:       LightSourceICARUS
// Module Type: producer
// File:        LightSourceICARUS_module.cc
//
// Generated at Sun Apr  9 10:40:23 2017 by Usher, Tracy L. using artmod
// from cetpkgsupport v1_11_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>

class LightSourceICARUS;

class LightSourceICARUS : public art::EDProducer {
public:
  explicit LightSourceICARUS(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  LightSourceICARUS(LightSourceICARUS const &) = delete;
  LightSourceICARUS(LightSourceICARUS &&) = delete;
  LightSourceICARUS & operator = (LightSourceICARUS const &) = delete;
  LightSourceICARUS & operator = (LightSourceICARUS &&) = delete;

  // Required functions.
  void produce(art::Event & e) override;

  // Selected optional functions.
  void beginJob() override;
  void beginRun(art::Run & r) override;
  void beginSubRun(art::SubRun & sr) override;
  void endJob() override;
  void endRun(art::Run & r) override;
  void endSubRun(art::SubRun & sr) override;
  void reconfigure(fhicl::ParameterSet const & p) override;
  void respondToCloseInputFile(art::FileBlock const & fb) override;
  void respondToCloseOutputFiles(art::FileBlock const & fb) override;
  void respondToOpenInputFile(art::FileBlock const & fb) override;
  void respondToOpenOutputFiles(art::FileBlock const & fb) override;

private:

  // Declare member data here.

};


LightSourceICARUS::LightSourceICARUS(fhicl::ParameterSet const & p)
// :
// Initialize member data here.
{
  // Call appropriate produces<>() functions here.
}

void LightSourceICARUS::produce(art::Event & e)
{
  // Implementation of required member function here.
}

void LightSourceICARUS::beginJob()
{
  // Implementation of optional member function here.
}

void LightSourceICARUS::beginRun(art::Run & r)
{
  // Implementation of optional member function here.
}

void LightSourceICARUS::beginSubRun(art::SubRun & sr)
{
  // Implementation of optional member function here.
}

void LightSourceICARUS::endJob()
{
  // Implementation of optional member function here.
}

void LightSourceICARUS::endRun(art::Run & r)
{
  // Implementation of optional member function here.
}

void LightSourceICARUS::endSubRun(art::SubRun & sr)
{
  // Implementation of optional member function here.
}

void LightSourceICARUS::reconfigure(fhicl::ParameterSet const & p)
{
  // Implementation of optional member function here.
}

void LightSourceICARUS::respondToCloseInputFile(art::FileBlock const & fb)
{
  // Implementation of optional member function here.
}

void LightSourceICARUS::respondToCloseOutputFiles(art::FileBlock const & fb)
{
  // Implementation of optional member function here.
}

void LightSourceICARUS::respondToOpenInputFile(art::FileBlock const & fb)
{
  // Implementation of optional member function here.
}

void LightSourceICARUS::respondToOpenOutputFiles(art::FileBlock const & fb)
{
  // Implementation of optional member function here.
}

DEFINE_ART_MODULE(LightSourceICARUS)