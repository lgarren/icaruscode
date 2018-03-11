#ifndef ICARUSHITFINDER_H
#define ICARUSHITFINDER_H

////////////////////////////////////////////////////////////////////
//HIT FINDER THAT RUNS ON RAW SIGNALS INSTEAD OF DECONVOLUTED ONES.
//DEVELOPED INITIALLY FOR ICARUS-T600 OLD ELECTRONICS AT LNGS
//filippo.varanini@pd.infn.it .
////////////////////////////////////////////////////////////////////

// C/C++ standard libraries
#include <string>
#include <vector>
#include <fstream>
#include <set>

//Framework
#include "fhiclcpp/ParameterSet.h" 
#include "messagefacility/MessageLogger/MessageLogger.h" 
#include "art/Framework/Core/ModuleMacros.h" 
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Principal/Event.h" 
#include "art/Framework/Principal/Handle.h" 
#include "canvas/Persistency/Common/Ptr.h" 
#include "art/Framework/Services/Registry/ServiceHandle.h" 
#include "canvas/Utilities/InputTag.h"
#include "canvas/Persistency/Common/FindOneP.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Utilities/ToolMacros.h"
#include "art/Utilities/make_tool.h"


//LArSoft
#include "larcore/Geometry/Geometry.h"
#include "larevt/Filters/ChannelFilter.h"
#include "lardata/Utilities/LArFFT.h"
#include "lardataobj/RawData/RawDigit.h"
#include "lardataobj/RawData/raw.h"
#include "lardataobj/RecoBase/Hit.h"
#include "lardata/ArtDataHelper/HitCreator.h"
#include "larevt/CalibrationDBI/Interface/ChannelStatusService.h"
#include "larevt/CalibrationDBI/Interface/ChannelStatusProvider.h"

//LArSoft From FFT
#include "larcoreobj/SimpleTypesAndConstants/geo_types.h"
#include "larcore/Geometry/Geometry.h"
#include "larcorealg/Geometry/CryostatGeo.h"
#include "larcorealg/Geometry/TPCGeo.h"
#include "larcorealg/Geometry/PlaneGeo.h"

#include "larreco/HitFinder/HitFinderTools/ICandidateHitFinder.h"
#include "icaruscode/HitFinder/HitFinderTools/IPeakFitter.h"

//ROOT from CalData
#include "TComplex.h"
#include "TFile.h"
#include "TH2D.h"
#include "TF1.h"

//ROOT From Gauss
#include "TH1D.h"
#include "TDecompSVD.h"
#include "TMath.h"

namespace hit {

  class ICARUSHitFinder : public art::EDProducer {

    public:

      explicit ICARUSHitFinder(fhicl::ParameterSet const& pset);
      virtual ~ICARUSHitFinder();

      void produce(art::Event& evt); 
      void beginJob(); 
      void endJob(); 
      void reconfigure(fhicl::ParameterSet const& p);
     
      
      struct ICARUSHit {
          int iWire;
          int iDrift;
          int iniDrift;
          int finDrift;
          double peakHeight; //raw hit height
          double iAdcMax; //fitted hit height
          int iwindow; //beginning of fitting window
          int fwindow;
          float tHit;
          float qHit;
          float chi2n;
          float qHitmax;
          float fitBaseline;
          float fitAmplitude;
          float fitT0;
          float fitTau1;
          float fitTau2;
          
          float fitWidth;
          float fitSlope;
          int fitwindowsize;
          float localmean;
      
      };
      
      void expandHit(ICARUSHit& h, std::vector<float> holder, std::vector<ICARUSHit> how );
      void computeBestLocalMean(ICARUSHit& h, std::vector<float> holder, std::vector<ICARUSHit> how);


    private:
      unsigned int  fDataSize;                  //SIZE OF RAW DATA ON ONE WIRE.
      art::InputTag fDigitModuleLabel;          //MODULE THAT MADE DIGITS.
      std::string   fSpillName;                 //NOMINAL SPILL IS AN EMPTY STRING.

      //FFT COPIED VARIABLES.
      std::string         fCalDataModuleLabel;
      std::string         fHitLabelName;
      double              fActiveInd1;
      double              fActiveInd2;
      double              fActiveCol;
      int          fInd1Width;            //INITIAL WIDTH FOR INDUCTION FIT.
      int              fInd2Width;            //INITIAL WIDTH FOR INDUCTION FIT.
      int              fColWidth;            //INITIAL WIDTH FOR COLLECTION FIT.
      unsigned int              fInd1Window;            //INITIAL WINDOW FOR INDUCTION FIT.
      unsigned int              fInd2Window;            //INITIAL WINDOW FOR INDUCTION FIT.
      unsigned int              fColWindow;            //INITIAL WINDOW FOR COLLECTION FIT.
      int              fInd1Threshold;            //THRESHOLD FOR INDUCTION FIT.
      int              fInd2Threshold;            //THRESHOLD FOR INDUCTION FIT.
      int              fColThreshold;            //THRESHOLD FOR COLLECTION FIT.
      int              fInd1Above;            //MINIMAL NUMBER OF TICKS ABOVE THRESHOLD FOR INDUCTION FIT.
      int              fInd2Above;
      int              fColAbove;
      int              fInd1Fall;
      int              fInd2Fall;
      int              fColFall;
      double              fInd1Multi;            //MULTI-FIT OF NEIGHBORRING HITS (still to implement)
      double              fInd2Multi;
      double              fColMulti;
      double              fInd1100kHz;            //REJECTION OF 100kHz NOISE (still to implement)
      double              fInd2100kHz;
      double              fCol100kHz;
      double              fInd1Isolated;            //REJECTION OF ISOLATED HITS
      double              fInd2Isolated;
      double              fColIsolated;
      double              fInd1RMS;            //CUT ON WIRE RMS
      double              fInd2RMS;
      double              fColRMS;
      int              fThetaAngle;
       bool                fUncompressWithPed;   //OPTION TO UNCOMPRESS WITH PEDESTAL.
      
      std::unique_ptr<reco_tool::ICandidateHitFinder> fHitFinderTool;  ///< For finding candidate hits
      std::unique_ptr<reco_tool::IPeakFitter>         fPeakFitterTool; ///< Perform fit to candidate peaks
      
      
      //histograms
      TH1F* fFirstChi2;
      TH1F* fChi2;
      TH1F* fHeightC;
      TH1F* fWidthC;
      TH1F* fNoiseC;
      TH1F* fAreaC;
      TH1F* fnhwC;
      TH1F* fIntegralC;
      TH1F* fAreaInt;


      TH1F* fHeightI2;
      TH1F* fWidthI2;
      TH1F* fNoiseI2;
      TH1F* fHeightI1;
      TH1F* fWidthI1;
      TH1F* fNoiseI1;
     
  }; // class ICARUSHitFinder


  //-------------------------------------------------
  ICARUSHitFinder::ICARUSHitFinder(fhicl::ParameterSet const& pset)
  {
    this->reconfigure(pset);

    //LET HITCOLLECTIONCREATOR DECLARE THAT WE ARE GOING TO PRODUCE
    //HITS AND ASSOCIATIONS TO RAW DIGITS BUT NOT ASSOCIATIONS TO WIRES
    //(WITH NO PARTICULAR PRODUCT LABEL).
    recob::HitCollectionCreator::declare_products(*this, 
        /*instance_name*/"");
  }

  //-------------------------------------------------
  ICARUSHitFinder::~ICARUSHitFinder()
  {
  }

  void ICARUSHitFinder::reconfigure(fhicl::ParameterSet const& p)
  {
      fUncompressWithPed  = p.get< bool         >("UncompressWithPed", true);
      fDigitModuleLabel   = p.get< art::InputTag >("DigitModuleLabel", "daq");
      fCalDataModuleLabel = p.get< std::string  >("CalDataModuleLabel");
      fActiveInd1         = p.get< double  >("ActiveInd1");
      fActiveInd2         = p.get< double  >("ActiveInd2");
      fActiveCol          = p.get< double  >("ActiveCol");
      fInd1Width          = p.get< int  >("Ind1Width");
      fInd2Width          = p.get< int  >("Ind2Width");
      fColWidth           = p.get< int  >("ColWidth");
      fInd1Window         = p.get< unsigned int  >("Ind1Window");
      fInd2Window         = p.get< unsigned int  >("Ind2Window");
      fColWindow          = p.get< unsigned int  >("ColWindow");
      fInd1Threshold      = p.get< int  >("Ind1Threshold");
      fInd2Threshold      = p.get< int  >("Ind2Threshold");
      fColThreshold       = p.get< int  >("ColThreshold");
      fInd1Above          = p.get< int  >("Ind1Above");
      fInd2Above          = p.get< int  >("Ind2Above");
      fColAbove           = p.get< int  >("ColAbove");
      fInd1Fall           = p.get< int  >("Ind1Fall");
      fInd2Fall           = p.get< int  >("Ind2Fall");
      fColFall            = p.get< int  >("ColFall");
      fInd1Multi          = p.get< double  >("Ind1Multi");
      fInd2Multi          = p.get< double  >("Ind2Multi");
      fColMulti           = p.get< double  >("ColMulti");
      fInd1100kHz         = p.get< double  >("Ind1100kHz");
      fInd2100kHz         = p.get< double  >("Ind2100kHz");
      fCol100kHz          = p.get< double  >("Col100kHz");
      fInd1Isolated       = p.get< double  >("Ind1Isolated");
      fInd2Isolated       = p.get< double  >("Ind2Isolated");
      fColIsolated        = p.get< double  >("ColIsolated");
      fInd1RMS            = p.get< double  >("Ind1RMS");
      fInd2RMS            = p.get< double  >("Ind2RMS");
      fColRMS             = p.get< double  >("ColRMS");
      fThetaAngle         = p.get< double  >("ThetaAngle");
      
      //fHitFinderTool  = art::make_tool<reco_tool::ICandidateHitFinder>(p.get<fhicl::ParameterSet>("CandidateHits"));
      //fPeakFitterTool = art::make_tool<reco_tool::IPeakFitter>(p.get<fhicl::ParameterSet>("PeakFitter"));
      
    mf::LogInfo("ICARUSHitFinder_module") << "fDigitModuleLabel: " << fDigitModuleLabel << std::endl;
  }

  //-------------------------------------------------
  void ICARUSHitFinder::beginJob()
  {
      // get access to the TFile service
      art::ServiceHandle<art::TFileService> tfs;
      
      
      // ======================================
      // === Hit Information for Histograms ===
      fFirstChi2	= tfs->make<TH1F>("fFirstChi2", "#chi^{2}", 10000, 0, 5000);
      fChi2	        = tfs->make<TH1F>("fChi2", "#chi^{2}", 10000, 0, 5000);
      fHeightC	= tfs->make<TH1F>("fHeightC", "height(ADC#)", 100, 0, 100);
      fWidthC	        = tfs->make<TH1F>("fWidthC", "width(samples)", 100, 0, 100);
      fNoiseC	        = tfs->make<TH1F>("fNoiseC", "Noise Area(ADC#)", 100, 0, 100);
      fAreaC            = tfs->make<TH1F>("fAreaC", "", 150, 0, 1500);
      fAreaC->GetXaxis()->SetTitle("Area(ADC#*ticks)");
      fAreaC->GetYaxis()->SetTitle("events");
      fIntegralC            = tfs->make<TH1F>("fIntegralC", "", 500, 0, 3000);
      fIntegralC->GetXaxis()->SetTitle("Area(ADC#*ticks)");
      fIntegralC->GetYaxis()->SetTitle("events");
      fAreaInt = tfs->make<TH1F>("fAreaInt", "Area/Int", 100,0.,2.);

      fnhwC            = tfs->make<TH1F>("fnhwC", "", 10, 0, 10);
      fnhwC->GetXaxis()->SetTitle("n hits per wire");
      fnhwC->GetYaxis()->SetTitle("wires");
      fHeightI2	= tfs->make<TH1F>("fHeightI2", "height(ADC#)", 100, 0, 100);
      fWidthI2	        = tfs->make<TH1F>("fWidthI2", "width(samples)", 100, 0, 100);
      fNoiseI2	        = tfs->make<TH1F>("fNoiseI2", "Noise Area(ADC#)", 100, 0, 100);
      fHeightI1	= tfs->make<TH1F>("fHeightI1", "height(ADC#)", 100, 0, 100);
      fWidthI1	        = tfs->make<TH1F>("fWidthI1", "width(samples)", 100, 0, 100);
      fNoiseI1	        = tfs->make<TH1F>("fNoiseI1", "Noise Area(ADC#)", 100, 0, 100);
  }

  void ICARUSHitFinder::endJob()
  {
      mf::LogDebug("ICARUSHitFinder") << " ICARUSHitFinder endjob " << std::endl;
   
  }

  //-------------------------------------------------
  void ICARUSHitFinder::produce(art::Event& evt)
  {      //0
      std::ofstream output("areaNoFit.out");
      
      //GET THE GEOMETRY.
      art::ServiceHandle<geo::Geometry> geom;
      
      // ###############################################
      // ### Making a ptr vector to put on the event ###
      // ###############################################
      // this contains the hit collection
      // and its associations to wires and raw digits
      
      // Handle the filtered hits collection...
      recob::HitCollectionCreator  hcol(*this, evt);

      //    if (fAllHitsInstanceName != "") filteredHitCol = &hcol;
      
      // ##########################################
      // ### Reading in the Wire List object(s) ###
      // ##########################################
      art::Handle< std::vector<recob::Wire> > wireVecHandle;
      evt.getByLabel(fCalDataModuleLabel,wireVecHandle);
      
      // #################################################################
      // ### Reading in the RawDigit associated with these wires, too  ###
      // #################################################################
      art::FindOneP<raw::RawDigit> RawDigits
      (wireVecHandle, evt, fCalDataModuleLabel);
      
      // Channel Number
      raw::ChannelID_t channel = raw::InvalidChannelID;
      
      std::vector<float> holder;      //HOLDS SIGNAL DATA.
      std::vector<short> rawadc;      //UNCOMPRESSED ADC VALUES.
      std::vector<float> startTimes;  //STORES TIME OF WINDOW START.
      std::vector<float> maxTimes;    //STORES TIME OF LOCAL MAXIMUM.
      std::vector<float> endTimes;    //STORES TIME OF WINDOW END.
      std::vector<float> peakHeight;  //STORES ADC COUNT AT THE MAXIMUM.
      std::vector<float> hitrms;    	 //STORES CHARGE WEIGHTED RMS OF TIME ACROSS THE HIT.
      std::vector<double> charge;     //STORES THE TOTAL CHARGE ASSOCIATED WITH THE HIT.
      
     // uint32_t channel   = 0;         //CHANNEL NUMBER.
      double   threshold = 0;         //MINIMUM SIGNAL SIZE FOR ID'ING A HIT.
      //double   totSig    = 0;
      //double   myrms     = 0;
      //double   mynorm    = 0;
      
      unsigned int window=0;
      int abovecut=0;
      int fall=0;
      int width=0;
      
      // geo::SigType_t sigType = geo::kInduction;
      std::stringstream numConv;
      
      unsigned int hwC(0),lwC(5728); //lowest and highest wire with physical deposition in Collection
      unsigned int hwI2(0),lwI2(5728); //same for Induction
      unsigned int hwI1(0),lwI1(2112);
      
      unsigned int nw1hitC(0),nw1hitI2(0),nw1hitI1(0); //number of wires (between lowest and highest) with a single hit (rough definition of hitfinding efficiency)
      int nhWire[5600];
      for(int jw=0;jw<5600;jw++)
          nhWire[jw]=0;
      float wCharge[5600];
      for(int jw=0;jw<5600;jw++)
      wCharge[jw]=0;
      float wInt[5600];
      for(int jw=0;jw<5600;jw++)
      wInt[jw]=0;
      
      unsigned int nhitsC(0),nhitsI1(0),nhitsI2(0); //total number of reconstructed hits in a view
      
      unsigned int nnhitsC(0),nnhitsI1(0),nnhitsI2(0); //total number of reconstructed hits in a view

      int minWireC,maxWireC;
      if(fThetaAngle==45) {minWireC=2539; maxWireC=3142;}
      if(fThetaAngle==0) {minWireC=2539; maxWireC=4700;}
      if(fThetaAngle==20) {minWireC=2539; maxWireC=4000;}
      if(fThetaAngle==40) {minWireC=2539; maxWireC=3190;}
      if(fThetaAngle==60) {minWireC=2539; maxWireC=2905;}
      if(fThetaAngle==70) {minWireC=2539; maxWireC=2805;}
      if(fThetaAngle==80) {minWireC=2539; maxWireC=2740;}
      
      //45 deg
      /*int minWireC=2539; //empirical
      int maxWireC=3142;
      int minWireI2=2539; //empirical
      int maxWireI2=3102;
      int minWireI1=0; //empirical
      int maxWireI1=497;*/
    
      int minWireI2=2539; //empirical
      int maxWireI2=4700;
      int minDrift=850;
      int maxDrift=1500;
      
      //### Looping over the wires ###
      //##############################
      for(size_t wireIter = 0; wireIter < wireVecHandle->size(); wireIter++)
      {
          // ####################################
          // ### Getting this particular wire ###
          // ####################################
          art::Ptr<recob::Wire>   wire(wireVecHandle, wireIter);
          art::Ptr<raw::RawDigit> rawdigits = RawDigits.at(wireIter);
          
          // --- Setting Channel Number and Signal type ---
          channel = wire->Channel();
          
          std::vector<float> signal(wire->Signal());
          
          // get the WireID for this hit
          std::vector<geo::WireID> wids = geom->ChannelToWire(channel);
          // for now, just take the first option returned from ChannelToWire
          geo::WireID wid  = wids[0];
          // We need to know the plane to look up parameters
          geo::PlaneID::PlaneID_t plane = wid.Plane;
          size_t cryostat=wid.Cryostat;
          size_t tpc=wid.TPC;

          holder.clear();

          //GET THE REFERENCE TO THE CURRENT raw::RawDigit.
          channel   = rawdigits->Channel();
          fDataSize = rawdigits->Samples();

          std::vector<geo::WireID> widVec = geom->ChannelToWire(channel);
          size_t                   iwire  = widVec[0].Wire;
     //   size_t plane = widVec[0].Plane;
        
          rawadc.resize(fDataSize);
          holder.resize(fDataSize);

          //UNCOMPRESS THE DATA.
          if (fUncompressWithPed) {
              int pedestal = (int)rawdigits->GetPedestal();
              raw::Uncompress(rawdigits->ADCs(), rawadc, pedestal, rawdigits->Compression());
          }
          else{
              raw::Uncompress(rawdigits->ADCs(), rawadc, rawdigits->Compression());
          }

          //GET THE LIST OF BAD CHANNELS.
          lariov::ChannelStatusProvider const& channelStatus
            = art::ServiceHandle<lariov::ChannelStatusService>()->GetProvider();

          lariov::ChannelStatusProvider::ChannelSet_t const BadChannels
            = channelStatus.BadChannels();

          for(unsigned int bin = 0; bin < fDataSize; ++bin){
              //holder[bin]=(rawadc[bin]-rawdigits->GetPedestal());
              holder[bin]=signal[bin];
              if(plane == 0) holder[bin]=-holder[bin];
              //if(plane == 1) holder[bin]=-holder[bin];
              if(plane==2&&iwire==3600&&abs(holder[bin])>0.1) mf::LogDebug("ICARUSHitFinder") << " col wire 3600 bin " << bin << " signal " << holder[bin] << std::endl;
              if(plane==1&&iwire==3600&&abs(holder[bin])>0.1) mf::LogDebug("ICARUSHitFinder") << " in2 wire 3600 bin " << bin << " signal " << holder[bin] << std::endl;
              if(plane==0&&iwire==600&&abs(holder[bin])>0.1) mf::LogDebug("ICARUSHitFinder") << " in1 wire 600 bin " << bin << " signal " << holder[bin] << std::endl;
          }

          if(plane==0&&iwire<lwI1) lwI1=iwire;
          if(plane==0&&iwire>hwI1) hwI1=iwire;
          if(plane==1&&iwire<lwI2) lwI2=iwire;
          if(plane==1&&iwire>hwI2) hwI2=iwire;
          if(plane==2&&iwire<lwC) lwC=iwire;
          if(plane==2&&iwire>hwC) hwC=iwire;
        
          // sigType = geom->SignalType(channel);

          peakHeight.clear();
          endTimes.clear();
          startTimes.clear();
          maxTimes.clear();
          charge.clear();
          hitrms.clear();
        
          bool channelSwitch = false;

          for(auto it = BadChannels.begin(); it != BadChannels.end(); it++)
          {
              if(channel==*it)
              {
                  channelSwitch = true;
                  break;
              }
          }

          if(channelSwitch==false)
          { //1
              // mf::LogDebug("ICARUSHitFinder") << " inside channel switch " << std::endl;
          
              if(plane == 0) threshold=fInd1Threshold;
              else if(plane == 1) threshold=fInd2Threshold;
              else if(plane == 2) threshold=fColThreshold;
              if(plane == 0) window=fInd1Window;
              else if(plane == 1) window=fInd2Window;
              else if(plane == 2) window=fColWindow;
              if(plane == 0) abovecut=fInd1Above;
              else if(plane == 1) abovecut=fInd2Above;
              else if(plane == 2) abovecut=fColAbove;
              if(plane == 0) fall=fInd1Fall;
              else if(plane == 1) fall=fInd2Fall;
              else if(plane == 2) fall=fColFall;
              if(plane == 0) width=fInd1Width;
              else if(plane == 1) width=fInd2Width;
              else if(plane == 2) width=fColWidth;

              //  mf::LogDebug("ICARUSHitFinder") << " col width " << fColWidth << std::endl;

              int iw=iwire;
              unsigned int i;
          
              int iflag;
              int localbellow,rising;
              int begin;
              // int nSamp=digitVec->Samples();
              int lastcomputedmean,count;
              int peakheight;
              int localminidx,localmin;
              // int jj, jcount;
              // float area;
              ICARUSHit h;
              std::vector<ICARUSHit> hits;
          
              // Hit finding parameters
              const int rise=5;
          
              // initialize parameters
              iflag=0;                // equal to one if we are within a hit candidate
              peakheight=-9999;       // last found hit maximum
              begin=-1;               // last found hit initial sample
              localbellow=0;          // number of times we are bellow peakheight
              lastcomputedmean=0;     // the value we compare with to know if we have a hit
              count=0;
              for(unsigned int j=0;j<window;j++)
                  count+=holder[j];
              lastcomputedmean=(count>=0)? 0 : count/window;
              //      negpeakwidth=0;
              //      if(lastcomputedmean<0) negpeakwidth++;
              localmin=9999;
              localminidx=-1;
              //     if ( typ == BasicData::ISBASICPLANE_PMT && iw==1 ){
              //cout << "window, mean " <<  window <<" "<<lastcomputedmean<<endl;
              //cout << "wire, nsamp " <<  iw <<" "<<nSamp<<endl;
              //}
              // loop on the selected samples
          
//          mf::LogDebug("ICARUSHitFinder") << " before loop fDataSize " << fDataSize << std::endl;
          
              for(i=0;i<fDataSize;i++)
              { //2
                  //mf::LogDebug("ICARUSHitFinder") << " i " << i << " holder " << holder[i] << std::endl;
                  // skip sharp peaks
                  //if(abs(holder[i]-lastcomputedmean)>100)
                    //  continue;
                  
                  if(!iflag)
                      lastcomputedmean=0;
                  
                  if(holder[i]-lastcomputedmean>threshold) // we're within a hit OR hit group
                  { //3
                  //  if(plane==0&&cryostat==0&&tpc==0)
                    //    mf::LogDebug("ICARUSHitFinder") << " iwire " << iwire <<"  over threshold bin " << i << std::endl;
                      iflag=1;
                      
                      // we're in the beginning of the hit
                      if(begin<0) {
                          
                          begin=i;     // hit starting point
                      }
                      
                      // keep peak info
                      if(holder[i]-lastcomputedmean>peakheight)
                      {
                          peakheight=holder[i]-lastcomputedmean;
                          h.iAdcMax=holder[i];
                          h.peakHeight=peakheight;
                          h.iWire=iw;
                          h.iDrift=i;
                          localbellow=0;
                      }
                      
                      // resolve close hits
                      //	      if(*(pf+i)-(peakheight+lastcomputedmean)<-threshold) // we're in the slope down
                      if(holder[i]-(peakheight+lastcomputedmean)<-1) // we're in the slope down
                      {
                          localbellow++;
                          //mf::LogDebug("ICARUSHitFinder") << " localbellow " << localbellow << " abovecut " << abovecut << std::endl;
                      }
                      if(localbellow>abovecut)
                      { //4
                          //if ( typ == BasicData::ISBASICPLANE_PMT && iw==1 )
                          //	     cout << "localbellow ab cut"<<i << endl;
                          // keep local minimum as border between consecutive hits
                          if(holder[i]<localmin) {localmin=holder[i];localminidx=i;}
                          
                          // count the number of rising samples within the following n=<rise> samples
                          rising=0;
                          for(int l=0;l<rise;l++)
                          {
                              if(i+l<fDataSize) {
                                  if(holder[i+l+1]-holder[i+l]>0) rising++;
                                  else if(holder[i+l+1]-holder[i+l]<0) rising--;
                              }
                          }
                          // if after a slope down there's a slope up save the previous hit
                          if(rising>abovecut)
                          { //5
                              h.iniDrift=begin;
                              //		      h.finDrift=i+iniSamp;
                              h.finDrift=localminidx;
                              if(cryostat==0&&tpc==0&&plane==2&&h.iWire==4326)
                                mf::LogDebug("ICARUSHitFinder") << "  before expand ini " << h.iniDrift << " fin " << h.finDrift << std::endl;
                             // expandHit(h,holder,hits);
                              
                              if((h.finDrift-h.iDrift)>=fall && h.finDrift-h.iniDrift>width)
                              { //6
                                  h.iwindow=h.iniDrift;
                                  h.fwindow=h.finDrift;
                                 // mf::LogDebug("ICARUSHitFinder") << " adding hit case 1" << std::endl;
                                  hits.push_back(h);
                                  peakheight=-9999;
                                  h.iAdcMax=0;
                                  //begin=i+iniSamp+1;
                                  begin=localminidx+1;
                                  localbellow=0;
                                  localminidx=-1;
                                  localmin=9999;
                              }
                          }
                      }
                  }
                  else // outside the hit
                  { //3
                      //if (  typ == BasicData::ISBASICPLANE_PMT  && iw==1){
                      //	cout << "out hit " << iflag << "iadc " << h.iAdcMax <<endl;
                      //	cout << " in, fin " << begin << " " << i+iniSamp << "peak "<<h.iDrift <<endl;}
                      if (iflag==1 && h.iAdcMax) //just getting out of the latest hit
                      { //4
                          h.iniDrift=begin;
                          h.finDrift=i;
                          if(iwire==4526&&plane==2&&cryostat==0&&tpc==0) {
                              //mf::LogDebug("ICARUSHitFinder") << " end hit  " << i << " fall? " << h.finDrift-h.iDrift <<" width? " << h.finDrift-h.iniDrift << std::endl;
//                               mf::LogDebug("ICARUSHitFinder") << " end hit  " << i << " fall par " << fall <<" width par " << width << std::endl;
                          }
                                  if((h.finDrift-h.iDrift)>=fall && (h.finDrift-h.iniDrift)>width)
                          { //5
                              h.iwindow=h.iniDrift;
                              h.fwindow=h.finDrift;
                           //if(iwire==4526&&plane==1&&cryostat==0&&tpc==0)
                            //mf::LogDebug("ICARUSHitFinder") << " adding hit case 2, tick " << i << std::endl;
                              if(cryostat==0&&tpc==0&&plane==2&&h.iWire==4326)
                              mf::LogDebug("ICARUSHitFinder") << "  before expand ini " << h.iniDrift << " fin " << h.finDrift << std::endl;
                              //expandHit(h,holder,hits);
                            
                              hits.push_back(h);
                                  //InsertHit(&h);
                              //  if ( typ != BasicData::ISBASICPLANE_PMT)
                            
                          }
                          
                          peakheight=-9999;
                          begin=-1;
                          iflag=0;
                          localbellow=0;
                      }
                  }
                          
                   //keep the last mean value to which we have to come back after the hit
                   if(i>=window && window) {
                       if(holder[i]-count/window>-10)
                       {
                           count+=holder[i];
                           count-=holder[i-window];
                           //		if((float) count/window<0) negpeakwidth++;
                           //		else negpeakwidth=0;
                       }
                   }
               } //end loop on samples
          
               //if we were within a hit while reaching last sample, keep it
               if(iflag==1 && h.iAdcMax) //just getting out of the latest hit
               { //3
                   h.iniDrift=begin;
                   h.finDrift=i-1;
                   
                   if((h.finDrift-h.iDrift)>=fall && (h.finDrift-h.iniDrift)>width)
                   {
                       if(cryostat==0&&tpc==0&&plane==2&&h.iWire==4326)
                       mf::LogDebug("ICARUSHitFinder") << "  before expand ini " << h.iniDrift << " fin " << h.finDrift << std::endl;
                      // expandHit(h,holder,hits);
               
                       h.iwindow=h.iniDrift;
                       h.fwindow=h.finDrift;
                       //mf::LogDebug("ICARUSHitFinder") << " adding hit case 3 " << i << std::endl;
                       hits.push_back(h);
                      // InsertHit(&h);
                   }
               }
          
              int    numHits(0);                       //NUMBER OF CONSECUTIVE HITS BEING FITTED.
              int    hitIndex(0);                      //INDEX OF CURRENT HIT IN SEQUENCE.
              double amplitude(0), position(0);        //FIT PARAMETERS.
              double start(0), end(0);
              double amplitudeErr(0), positionErr(0);  //FIT ERRORS.
              double goodnessOfFit(0), chargeErr(0);   //CHI2/NDF and error on charge.
              double hrms(0);
              double totSig(0);
              double intSig(0);

              std::vector<geo::WireID> wids = geom->ChannelToWire(channel);
       //   geo::WireID wid = wids[0];

              numHits = hits.size();
              int nghC=0;
              int nghI2=0;
              int nghI1=0;
          
              if(cryostat==0&&tpc==0&&plane!=2)
                  mf::LogDebug("ICARUSHitFinder") << " plane " << plane <<  "  Wire " << iwire << " numhits " << numHits << std::endl;
              for (int i = 0; i < numHits; i++) {
                  expandHit(hits[i],holder,hits);
                  hits[i].iwindow=hits[i].iniDrift;
                  hits[i].fwindow=hits[i].finDrift;
              }
              for (int i = 0; i < numHits; i++)
              {
                  if(cryostat==0&&tpc==0&&plane==1)
                      mf::LogDebug("ICARUSHitFinder") << "  middle ind hit " << hits[i].iniDrift << " fin " << hits[i].finDrift << std::endl;
                  totSig=0;
                  intSig=0;
                  // if(cryostat!=0||tpc!=0) continue;
          
                  amplitude     = hits[i].peakHeight;
                  position      = hits[i].iDrift;
                  start         = hits[i].iniDrift;
                  end           = hits[i].finDrift;
                  //temporary definition of hrms for ICARUS non-Gaussian hits
                  hrms          = (end-start)/3.;
                  amplitudeErr  = -1;
                  positionErr   = 1.0;
                  goodnessOfFit = -1;
                  chargeErr     = -1;
          
                  computeBestLocalMean(hits[i],holder,hits);
                  //hits[i].localmean=0;
          
                  // totSig        = std::accumulate(holder.begin() + (int) start, holder.begin() + (int) end, 0.);
                  for(unsigned int js=start;js<=end;js++)
                      totSig+=(holder[js]-hits[i].localmean);
                  if(!i)
                      for(unsigned int js=0;js<=4095;js++)
                          intSig+=(holder[js]-hits[i].localmean);
                  // if(plane==2&&hits[i].iWire==4451)
                  //   for(unsigned int js=0;js<=4095;js++)
                  //     mf::LogDebug("ICARUSHitFinder") << " tick " << js << " signal " << holder[js] << " localmean " << hits[i].localmean << " intSig " << holder[js]-hits[i].localmean << std::endl;
                  // if(cryostat==0&&tpc==0&&plane==2&&hits[i].localmean!=0)
                  //mf::LogDebug("ICARUSHitFinder") << " wire " << hits[i].iWire << " localmean " << hits[i].localmean <<std::endl;
          
                  //mf::LogDebug("ICARUSHitFinder") << " before hit creator " << std::endl;
                  recob::HitCreator hit(
                                        *wire,                                                                     //RAW DIGIT REFERENCE.
                                        wid,                                                                           //WIRE ID.
                                        start,                                                                         //START TICK.
                                        end,                                                                           //END TICK.
                                        hrms,                                                                          //RMS.
                                        position,                                                                      //PEAK_TIME.
                                        positionErr,                                                                   //SIGMA_PEAK_TIME.
                                        amplitude,                                                                     //PEAK_AMPLITUDE.
                                        amplitudeErr,                                                                  //SIGMA_PEAK_AMPLITUDE.
                                        totSig,                                                                        //HIT_INTEGRAL.
                                        chargeErr,                                                                     //HIT_SIGMA_INTEGRAL.
                                        std::accumulate(holder.begin() + (int) start, holder.begin() + (int) end, 0.), //SUMMED CHARGE.
                                        1,                                                                             //MULTIPLICITY.
                                        -1,                                                                            //LOCAL_INDEX.
                                        goodnessOfFit,                                                                 //WIRE ID.
                                        int(end-start+1)                                                               //DEGREES OF FREEDOM.
                                        );
          
                  // mf::LogDebug("ICARUSHitFinder") << " before emplace back " << std::endl;
                  //       filteredHitVec.push_back(hit.copy());
          
                  hcol.emplace_back(hit.move(), wire, rawdigits);
          
                  // bool driftWindow=hits[i].iDrift>=minDrift&&hits[i].iDrift<=maxDrift;
                  bool wireWindowC=hits[i].iWire>=minWireC&&hits[i].iWire<=maxWireC;
                  bool wireWindowI2=hits[i].iWire>=minWireI2&&hits[i].iWire<=maxWireI2;
                  //bool wireWindowI1=hits[i].iWire>=minWireI1+200&&hits[i].iWire<=maxWireI1-200;
          
                  bool outDriftWindow=hits[i].iDrift<=minDrift||hits[i].iDrift>=maxDrift;
                  bool outWireWindowC=hits[i].iWire<=minWireC||hits[i].iWire>=maxWireC;
                  bool outWireWindowI2=hits[i].iWire<=minWireI2||hits[i].iWire>=maxWireI2;
                  //bool outWireWindowI1=hits[i].iWire<=minWireI1-200||hits[i].iWire>=maxWireI1+200;
          
                  // if(plane==2)
                  //   mf::LogDebug("ICARUSHitFinder") << " wire " << hits[i].iWire << " drift " << hits[i].iDrift << " driftwindow " << driftWindow << " wireWindowI2 " << wireWindowI2 << std::endl;
          
                  if(plane==0)  {
                      //   mf::LogDebug("ICARUSHitFinder") << " wire " << hits[i].iWire << " ngh " << ngh << std::endl;
                      nghI1++;
                      if(nghI1==1) nw1hitI1++;
              
                      //  mf::LogDebug("ICARUSHitFinder") << " wire " << hits[i].iWire << " nw1h " << nw1 << std::endl;
                      fHeightI1->Fill(amplitude);
                      fWidthI1->Fill(end-start);
                  }
                  if(plane==1&&wireWindowI2) {
                      //   mf::LogDebug("ICARUSHitFinder") << " wire " << hits[i].iWire << " ngh " << ngh << std::endl;
                      nghI2++;
                      if(nghI2==1) nw1hitI2++;
                      // mf::LogDebug("ICARUSHitFinder") << " filling height histo wire" << hits[i].iWire << " drift " << hits[i].iDrift << " amplitude " << amplitude << std::endl;
                      fHeightI2->Fill(amplitude);
                      fWidthI2->Fill(end-start);
                  }
                  if(plane==2&&wireWindowC) {
                      nghC++;
                      if(nghC==1) nw1hitC++;
                      nhWire[hits[i].iWire]++;
                      fHeightC->Fill(amplitude);
                      fWidthC->Fill(end-start);
                      // fAreaC->Fill(totSig);
                      wCharge[hits[i].iWire]+=totSig;
                      wInt[hits[i].iWire]+=intSig;
              
                      if(intSig<0.&&hits[i].iWire==4451)
                          mf::LogDebug("ICARUSHitFinder") << "  intsig " << intSig << " localmean " << hits[i].localmean << " wire " << hits[i].iWire << std::endl;
                  }
          
                  if(plane==0) nhitsI1++;
                  if(plane==1) nhitsI2++;
                  if(plane==2) nhitsC++;
          
                  if(plane==0&&tpc==0&&cryostat==0&&outDriftWindow) {nnhitsI1++; fNoiseI1->Fill(amplitude); }
                  if(plane==1&&tpc==0&&cryostat==0&&outWireWindowI2) { nnhitsI2++; fNoiseI2->Fill(amplitude);
                      if(cryostat==0&&tpc==0)
                          mf::LogDebug("ICARUSHitFinder") << " noise hit Wire " << hits[i].iWire << " tick " << hits[i].iDrift << " nnhit2 " << nnhitsI2 <<  std::endl;
                  }
                  if(plane==2&&tpc==0&&cryostat==0&&outWireWindowC) { nnhitsC++; fNoiseC->Fill(amplitude); }
          
                  ++hitIndex;
              } //end loop on found hits
              if(plane==2&&cryostat==0&&tpc==0&&wCharge[iwire]>0.)
                  mf::LogDebug("ICARUSHitFinder") << " filling  wire  " << iwire << " area " << wCharge[iwire] << std::endl;
              if(plane==2&&cryostat==0&&tpc==0)
                  if(wCharge[iwire]>0)
                      fAreaC->Fill(wCharge[iwire]);
              if(plane==2&&cryostat==0&&tpc==0)
                  if(wCharge[iwire]>0)
                      output << iwire << " " <<wCharge[iwire] << std::endl;
              if(plane==2&&cryostat==0&&tpc==0)
                  if(wInt[iwire]>0)
                      fIntegralC->Fill(wInt[iwire]);
              if(wInt[iwire]>0&&cryostat==0&&tpc==0)
                  fAreaInt->Fill(wCharge[iwire]/wInt[iwire]);
            //if(wCharge[iwire]>0&&cryostat==0&&tpc==0&&wCharge[iwire]/wInt[iwire]<0.9)
              //  mf::LogDebug("ICARUSHitFinder") << " wire " << iwire << " ratio " << wCharge[iwire]/wInt[iwire] << std::endl;
          } //end channel condition

      } //end loop on channels
      
      double PhysWC=maxWireC-minWireC+1;
      double NoiseWC=5119-480-(maxWireC-minWireC+1);
      double PhysWI2=maxWireI2-minWireI2+1;
      double NoiseWI2=5119-480-(maxWireI2-minWireI2+1);
      double PhysWI1=1056;
      double NoiseWI1=1056;
    
      mf::LogDebug("ICARUSHitFinder") << " Physical collection wires " << PhysWC << " single hit wires " << nw1hitC << " efficiency " << float(nw1hitC)/PhysWC << std::endl;
      mf::LogDebug("ICARUSHitFinder") << " Average collection noise hits per wire " << nnhitsC << " " << NoiseWC << std::endl;
      mf::LogDebug("ICARUSHitFinder") << " Physical ind2 wires " << PhysWI2 << " single hit wires " << nw1hitI2 << " noise " << nnhitsI2 << " efficiency " << float(nw1hitI2)/PhysWI2 << std::endl;
      mf::LogDebug("ICARUSHitFinder") << " Average ind2 noise hits per wire " << nnhitsI2<< " " << NoiseWI2 << std::endl;
      mf::LogDebug("ICARUSHitFinder") << " Physical ind1 wires " << PhysWI1 << " single hit wires " << nw1hitI1 << " efficiency " << float(nw1hitI1)/PhysWI1 << std::endl;
      mf::LogDebug("ICARUSHitFinder") << " Average ind1 noise hits per wire " << nnhitsI1/NoiseWI1 << std::endl;

      for(int jw=minWireC;jw<maxWireC;jw++)
          fnhwC->Fill(nhWire[jw]);
      
      
      hcol.put_into(evt);
        mf::LogDebug("ICARUSHitFinder") << " end ICARUSHitfinder " << std::endl;
  } //end produce

    
    void ICARUSHitFinder::expandHit(ICARUSHit& h, std::vector<float> holder, std::vector<ICARUSHit> how)
    {
        // Given a hit or hit candidate <hit> expand its limits to the closest minima
        int nsamp=50;
        int cut=1;
        int upordown;
        int found=0;
        
        int first=h.iniDrift;
        int last =h.finDrift;
        
        std::vector<ICARUSHit>::iterator hiter;
        std::vector<ICARUSHit> hlist;
        
        // fill list of existing hits on this wire
        for(unsigned int j=0;j<how.size();j++)
        {
            ICARUSHit h2=how[j];
            if(h2.iDrift!=h.iDrift)
            hlist.push_back(h2);
        }
        
        
        // look for first sample
        while(!found)
        {
            if(first==0)
                break;
            
            for(hiter=hlist.begin();hiter!=hlist.end();hiter++)
            {
                ICARUSHit h2=*hiter;
                if(first==h2.finDrift)
                  found=1;
            }
            
            if(found==1) break;
            
            upordown=0;
            for(int l=0;l<nsamp/2;l++)
            {
                if(first-nsamp/2+l<4095) {
                    if(holder[first-nsamp/2+l+1]-holder[first-nsamp/2+l]>0) upordown++;
                    else if(holder[first-nsamp/2+l+1]-holder[first-nsamp/2+l]<0) upordown--;
            } }
           // mf::LogDebug("ICARUSHitFinder") << " checking " << first << " upordown " << upordown << std::endl;
            if(upordown>cut)
                first--;
            else
                found=1;
        }
        
        // look for last sample
        found=0;
        while(!found)
        {
            if(last==4095)
            {
                found=1;
                break;
            }
            
            for(hiter=hlist.begin();hiter!=hlist.end();hiter++)
            {
                ICARUSHit h2=*hiter;
                if(last==h2.iniDrift)
                found=1;
             //   mf::LogDebug("ICARUSHitFinder") << " last " << last << " h2ini " << h2.iniDrift << " found " << found << std::endl;
            }
            
            if(found==1) break;
            
            upordown=0;
            for(int l=0;l<nsamp/2;l++)
            {
                if(last+nsamp/2-l>0) {
                    if(holder[last+nsamp/2-l]-holder[last+nsamp/2-l-1]>0) upordown++;
                    else if(holder[last+nsamp/2-l]-holder[last+nsamp/2-l-1]<0) upordown--;
                } }
            
            if(upordown<-cut)
                last++;
            else
                found=1;
        }
        
        h.iniDrift=first;
        h.finDrift=last;
    }
    void ICARUSHitFinder::computeBestLocalMean(ICARUSHit& h, std::vector<float> holder, std::vector<ICARUSHit> how)
    {
        const int bigw=130;   //size of the window where to look for the minimum localmean value
        const int meanw=70;   //size of the window where the mean is calculated
        const int outofbounds=9999;
        
        float samples1[bigw];   //list to contain samples bellow the inidrift
        float samples2[bigw];   //list to contain samples above the findrift
        
        std::vector<ICARUSHit>::iterator hiter;
        std::vector<ICARUSHit> hlist;

        float min1;
        float min2;
        int inidrift=h.iniDrift;
        int findrift=h.finDrift;
        float count1=0;
        float count2=0;
        int shift1=0;
        int shift2=0;
        int foundborder1=0;
        int foundborder2=0;
        
        // fill list of existing hits on this wire
        for(unsigned int j=0;j<how.size();j++)
        {
            ICARUSHit h2=how[j];
            if(h2.iDrift!=h.iDrift)
            hlist.push_back(h2);
        }
        
        // fill the arrays of samples to be examined
        for(int i=0;i<bigw;i++)
        {
            // remove samples from other hits in the wire
            for(hiter=hlist.begin();hiter!=hlist.end();hiter++)
            {
                ICARUSHit h2=*hiter;
                if(inidrift-i-shift1 >= h2.iniDrift && inidrift-i-shift1 <= h2.finDrift)
                shift1+=h2.finDrift-h2.iniDrift+1;
                else if(findrift+i+shift2 >= h2.iniDrift && findrift+i+shift2 <= h2.finDrift)
                shift2+=h2.finDrift-h2.iniDrift+1;
            }
            
            // fill the lists
            if(inidrift-i-shift1>=0)
            samples1[i]=holder[h.iniDrift-i-shift1];
            else
            samples1[i]=outofbounds;
            
            if(findrift+i+shift2<=4095)
            samples2[i]=holder[h.finDrift+i+shift2];
            else
            samples2[i]=outofbounds;
        }
        
        // initialize counters
        for(int j=0;j<meanw;j++)
        {
            if(samples1[j]==outofbounds) foundborder1=1;
            if(samples2[j]==outofbounds) foundborder2=1;
            
            if(!foundborder1) count1+=samples1[j];
            if(!foundborder2) count2+=samples2[j];
        }
        min1=count1;
        min2=count2;
        
        //look for the local minima from the filled lists
        for(int j=0;j<bigw-meanw;j++)
        {
            if(count1<min1) min1=count1;
            if(samples1[j+meanw]==outofbounds) foundborder1=1;
            if(!foundborder1) count1+=samples1[j+meanw]-samples1[j];
            
            if(count2<min2) min2=count2;
            if(samples2[j+meanw]==outofbounds) foundborder2=1;
            if(!foundborder2) count2+=samples2[j+meanw]-samples2[j];
            
            if(foundborder1 && foundborder2) break;
        }
        if(count1<min1) min1=count1;
        if(count2<min2) min2=count2;
        
       // mf::LogDebug("ICARUSHitFinder") << " localmean count1 " << count1 << " count2 " << count2 << std::endl;
        //for the moment take the highest value
        if((foundborder1 && !foundborder2)  || (shift1 && !shift2))
        h.localmean=((float) min2)/meanw;
        else if((!foundborder1 && foundborder2) || (!shift1 && shift2))
        h.localmean=((float) min1)/meanw;
        else
        h.localmean=(min1>min2)?((float) min1)/meanw :((float) min2)/meanw;
        
        //for the moment take the average value
        //  h->localmean=(min1+min2)/(2.*meanw);
        
        
    }

  
  DEFINE_ART_MODULE(ICARUSHitFinder)   

} // end namespace hit


#endif //ICARUSHitFinder_H
