#ifndef NTUPLIZER_H
#define NTUPLIZER_H

#include <cmath>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <utility>
#include <TNtuple.h>


#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include <FWCore/Framework/interface/Frameworkfwd.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/Framework/interface/ESHandle.h>
#include <FWCore/Utilities/interface/InputTag.h>
#include <DataFormats/PatCandidates/interface/Muon.h>
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Common/interface/TriggerNames.h"


#include "CommonTools/UtilAlgos/interface/TFileService.h"


class Ntuplizer : public edm::EDAnalyzer {
    public:
        /// Constructor
        explicit Ntuplizer(const edm::ParameterSet&);
        /// Destructor
        virtual ~Ntuplizer();  
        
    private:
        //----edm control---
        virtual void beginJob() ;
        virtual void analyze(const edm::Event&, const edm::EventSetup&);
        virtual void endJob() ;
        void Initialize(); 
        
        TTree *_tree;
        std::string _treeName;
        // -------------------------------------
        // variables to be filled in output tree
        ULong64_t       _indexevents;
        Int_t           _runNumber;
        Int_t           _lumi;

        edm::EDGetTokenT<pat::MuonRefVector>  _muonsTag;
        edm::EDGetTokenT<pat::TriggerObjectStandAloneCollection> _triggerObjects;
        edm::EDGetTokenT<edm::TriggerResults> _triggerBits;

        std::vector<Float_t> _muonsPtVector;
        
};


// ----Constructor and Destructor -----
Ntuplizer::Ntuplizer(const edm::ParameterSet& iConfig) :
_muonsTag(consumes<pat::MuonRefVector>(iConfig.getParameter<edm::InputTag>("muons"))),
_triggerObjects(consumes<pat::TriggerObjectStandAloneCollection>(iConfig.getParameter<edm::InputTag>("triggerSet"))),
_triggerBits(consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("triggerResultsLabel")))
{
    this -> _treeName = iConfig.getParameter<std::string>("treeName");
    this -> Initialize();
    return;
}

Ntuplizer::~Ntuplizer()
{}


void Ntuplizer::Initialize() {
    this -> _indexevents = 0;
    this -> _runNumber = 0;
    this -> _lumi = 0;

    this -> _muonsPtVector.clear();    
}


void Ntuplizer::beginJob()
{
    edm::Service<TFileService> fs;
    this -> _tree = fs -> make<TTree>(this -> _treeName.c_str(), this -> _treeName.c_str());

    //Branches
    this -> _tree->Branch("EventNumber",&_indexevents,"EventNumber/l");
    this -> _tree->Branch("RunNumber",&_runNumber,"RunNumber/I");
    this -> _tree->Branch("lumi",&_lumi,"lumi/I");

    this -> _tree->Branch("muonsPt", &_muonsPtVector);
    return;
}


void Ntuplizer::endJob()
{
    return;
}


void Ntuplizer::analyze(const edm::Event& iEvent, const edm::EventSetup& eSetup)
{

    this -> Initialize();

    _indexevents = iEvent.id().event();
    _runNumber = iEvent.id().run();
    _lumi = iEvent.luminosityBlock();

    std::auto_ptr<pat::MuonRefVector> resultMuon(new pat::MuonRefVector);

    // search for the tag in the event
    edm::Handle<pat::MuonRefVector> muonHandle;
    edm::Handle<pat::TriggerObjectStandAloneCollection> triggerObjects;
    edm::Handle<edm::TriggerResults> triggerBits;

    iEvent.getByToken(this -> _muonsTag, muonHandle);
    iEvent.getByToken(this -> _triggerObjects, triggerObjects);
    iEvent.getByToken(this -> _triggerBits, triggerBits);

    const edm::TriggerNames &names = iEvent.triggerNames(*triggerBits);

    for (pat::TriggerObjectStandAlone obj : *triggerObjects)
    {
        obj.unpackPathNames(names);
        const edm::TriggerNames::Strings& triggerNames = names.triggerNames();
        for (unsigned int x = 0; x < names.size(); x++)
        {
            bool isTriggered = false;
            if (triggerNames[x].find("HLT_DoubleMediumIsoPFTau32_Trk1_eta2p1_Reg_v") != std::string::npos) isTriggered = true;
            if (triggerNames[x].find("HLT_DoubleMediumIsoPFTau35_Trk1_eta2p1_Reg_v") != std::string::npos) isTriggered = true;
            if (triggerNames[x].find("HLT_DoubleMediumIsoPFTau40_Trk1_eta2p1_Reg_v") != std::string::npos) isTriggered = true;
        }
    }

    for (size_t imu = 0; imu < muonHandle -> size(); ++imu )
    {
        const pat::MuonRef mu = (*muonHandle)[imu] ;
        std::cout << "##### MUON PT: " << mu -> pt() << std::endl;
        this -> _muonsPtVector.push_back(mu -> pt());
    }

    this -> _tree -> Fill();
    
}

#include <FWCore/Framework/interface/MakerMacros.h>
DEFINE_FWK_MODULE(Ntuplizer);

#endif //NTUPLIZER_H
