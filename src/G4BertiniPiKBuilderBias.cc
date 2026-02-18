#include "G4BertiniPiKBuilderBias.hh"
#include <G4SystemOfUnits.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>
#include <G4ProcessManager.hh>
#include <G4BGGPionInelasticXS.hh>
#include <G4ComponentGGHadronNucleusXsc.hh>
#include <G4CrossSectionInelastic.hh>
#include <G4HadronicParameters.hh>
#include <G4PionPlus.hh>
#include <G4PionMinus.hh>
#include <G4KaonPlus.hh>


G4BertiniPiKBuilderBias::
G4BertiniPiKBuilderBias(double pion_plus_bias, double kaon_plus_bias)
  : fPionPlusBias(pion_plus_bias), fKaonPlusBias(kaon_plus_bias) {
   kaonxs = new G4CrossSectionInelastic( new G4ComponentGGHadronNucleusXsc );
   theMin = 0.0;
   theMax = G4HadronicParameters::Instance()->GetMaxEnergyTransitionFTF_Cascade();
   theModel = new G4CascadeInterface;
   theModel->SetMinEnergy(theMin);
   theModel->SetMaxEnergy(theMax);
 }

void G4BertiniPiKBuilderBias::
Build(G4HadronInelasticProcess * aP)
 {
   theModel->SetMinEnergy(theMin);
   theModel->SetMaxEnergy(theMax);
   const G4ParticleDefinition* particle = aP->GetParticleDefinition();
   if (particle == G4PionPlus::Definition()) {
     aP->AddDataSet( new G4BGGPionInelasticXS( G4PionPlus::Definition() ) );
     std::cout << "Biasing PiPlus: " << fPionPlusBias << std::endl;
     aP->MultiplyCrossSectionBy(fPionPlusBias);
   } else if (particle == G4PionMinus::Definition()) {
     aP->AddDataSet( new G4BGGPionInelasticXS( G4PionMinus::Definition() ) );
   } else if (particle == G4KaonPlus::Definition()) {
     aP->AddDataSet(kaonxs);
     std::cout << "Multiplying kaon by " << fKaonPlusBias << std::endl;
     aP->MultiplyCrossSectionBy(fKaonPlusBias);
   } else {
     aP->AddDataSet(kaonxs);
   }
   aP->RegisterMe(theModel);
 }
