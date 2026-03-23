/// EDepSimPhotonSD.cc
/// Optical-photon sensitive detector for the LArTPC geometry.
///
/// ProcessHits() is called by Geant4 for every step that overlaps with the
/// sensitive volume.  Geant4's transportation always stops a step at volume
/// boundaries, so a photon cannot skip entirely through a thin SD — it will
/// always generate at least one step entering the volume.
///
/// Two step geometries are handled:
///   (a) Normal step: pre-step already inside the sensor.
///       Pre-step touchable = sensor volume.
///   (b) Boundary-crossing step: pre-step is in LAr (outside), post-step
///       is inside the sensor.  Geant4 still calls ProcessHits because the
///       step endpoint is inside the SD.  Pre-step touchable = LAr volume,
///       post-step touchable = sensor volume.
///
/// In both cases we:
///   1. Kill the track immediately (fStopAndKill) — perfect absorber.
///   2. Record the hit using the pre-step position (photon entry point).
///   3. Use the G4TouchableHistory* argument for the copy number — Geant4
///      guarantees this is always valid and refers to the SD volume, avoiding
///      the null-pointer risk of dereferencing the post-step touchable handle
///      at a GeomBoundary step.
///
/// The fStopAndKill guard at the top prevents double-recording if Geant4
/// re-enters the SD after the status is already set.

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

G4bool EDepSim::PhotonSD::ProcessHits(G4Step* step, G4TouchableHistory* touch) {
    G4Track* track = step->GetTrack();

    // Only handle optical photons.
    if (track->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition())
        return false;

    // Guard: if already killed by a previous ProcessHits call for this track
    // (e.g. Geant4 re-enters the SD after we set fStopAndKill), skip.
    if (track->GetTrackStatus() == fStopAndKill)
        return false;

    // Kill the track — the sensor is a perfect absorber.
    track->SetTrackStatus(fStopAndKill);

    G4StepPoint* pre = step->GetPreStepPoint();

    EDepSim::PhotonHit* hit = new EDepSim::PhotonHit();

    // Record the pre-step position: the photon's position when it first
    // touches the sensor, regardless of whether it entered mid-step.
    hit->position  = pre->GetPosition();
    hit->direction = track->GetMomentumDirection();
    hit->time      = pre->GetGlobalTime() / CLHEP::ns;  // ns
    hit->energy    = track->GetKineticEnergy();          // MeV

    // Wavelength in nm: λ = hc / E
    // hc = 1239.84 eV·nm; energy in MeV → eV = energy * 1e6
    double energy_eV = hit->energy * 1.0e6;
    hit->wavelength  = (energy_eV > 0.0) ? (1239.84 / energy_eV) : 0.0; // nm

    // Use the G4TouchableHistory* provided by Geant4 for the copy number.
    // This is always valid and always refers to the SD volume being hit,
    // covering both the normal-step and boundary-crossing-step cases safely.
    // Avoids dereferencing post-step touchable handles which can be null
    // at a GeomBoundary step.
    hit->copyNo = touch ? touch->GetCopyNumber() : -1;

    fHits->insert(hit);
    return true;
}

void EDepSim::PhotonSD::EndOfEvent(G4HCofThisEvent*) {
    EDepSimLog("PhotonSD: " << fHits->entries() << " photon hits this event");
}
