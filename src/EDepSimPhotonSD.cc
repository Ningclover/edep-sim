/// EDepSimPhotonSD.cc
/// Optical-photon sensitive detector for the LArTPC geometry.
///
/// ProcessHits() is called by Geant4 for every step taken inside the
/// sensitive volume.  For an optical photon the absorbing step is the one
/// where the track status becomes fStopAndKill (absorption or detection
/// process).  We record that step and ignore all others.

#include "EDepSimPhotonSD.hh"
#include "EDepSimLog.hh"

#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4SDManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

EDepSim::PhotonSD::PhotonSD(G4String name)
    : G4VSensitiveDetector(name), fHits(nullptr), fHCID(-1)
{
    collectionName.insert("PhotonHits");
}

EDepSim::PhotonSD::~PhotonSD() {}

void EDepSim::PhotonSD::Initialize(G4HCofThisEvent* HCE) {
    fHits = new PhotonHitCollection(GetName(), GetCollectionName(0));
    if (fHCID < 0) {
        G4String hcName = GetName() + "/" + GetCollectionName(0);
        fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(hcName);
        EDepSimLog("PhotonSD initialised: " << hcName);
    }
    HCE->AddHitsCollection(fHCID, fHits);
}

G4bool EDepSim::PhotonSD::ProcessHits(G4Step* step, G4TouchableHistory*) {
    G4Track* track = step->GetTrack();

    // Only handle optical photons.
    if (track->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition())
        return false;

    // Only record the final (absorbing) step so we get one hit per photon.
    if (track->GetTrackStatus() != fStopAndKill)
        return false;

    G4StepPoint* post = step->GetPostStepPoint();

    EDepSim::PhotonHit* hit = new EDepSim::PhotonHit();

    hit->position  = post->GetPosition();          // mm
    hit->direction = track->GetMomentumDirection();
    hit->time      = post->GetGlobalTime() / CLHEP::ns;  // ns
    hit->energy    = track->GetKineticEnergy();    // MeV

    // Wavelength in nm: λ = hc / E
    // hc = 1239.84 eV·nm; energy in MeV → eV = energy * 1e6
    double energy_eV = hit->energy * 1.0e6;
    hit->wavelength  = (energy_eV > 0.0) ? (1239.84 / energy_eV) : 0.0; // nm

    // Copy number of the physical volume (sensor tile index 0-24)
    hit->copyNo = step->GetPreStepPoint()
                      ->GetTouchableHandle()
                      ->GetCopyNumber();

    fHits->insert(hit);
    return true;
}

void EDepSim::PhotonSD::EndOfEvent(G4HCofThisEvent*) {
    EDepSimLog("PhotonSD: " << fHits->entries() << " photon hits this event");
}
