////////////////////////////////////////////////////////////
// $Id: EDepSim::UserTrackingAction.cc,v 1.6 2007/01/01 05:32:49 mcgrew Exp $
//
#include "EDepSimUserTrackingAction.hh"
#include "EDepSimTrajectory.hh"
#include "EDepSimTrajectoryMap.hh"
#include "EDepSimPhotonManager.hh"
#include "EDepSimLog.hh"

#include "globals.hh"
#include "G4Track.hh"
#include "G4TrackingManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4VSensitiveDetector.hh"

EDepSim::UserTrackingAction::UserTrackingAction() {}

EDepSim::UserTrackingAction::~UserTrackingAction() {}

void EDepSim::UserTrackingAction::PreUserTrackingAction(const G4Track* trk) {
    EDepSimTrace("Pre-tracking action");
    G4VTrajectory* traj = new EDepSim::Trajectory(trk);
    fpTrackingManager->SetTrajectory(traj);
    fpTrackingManager->SetStoreTrajectory(true);
    EDepSim::TrajectoryMap::Add(traj);
}

void EDepSim::UserTrackingAction::PostUserTrackingAction(const G4Track* trk) {
    // Record every optical photon when it dies, regardless of where it ends up.
    if (trk->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition())
        return;

    G4cout << "[DBG] PostUserTrackingAction: start" << G4endl;

    TG4PhotonHit hit;

    G4cout << "[DBG] getting position" << G4endl;
    const G4ThreeVector& pos = trk->GetPosition();

    G4cout << "[DBG] getting direction" << G4endl;
    const G4ThreeVector& dir = trk->GetMomentumDirection();

    G4cout << "[DBG] setting Position/Direction" << G4endl;
    hit.Position.SetXYZ(pos.x(), pos.y(), pos.z());
    hit.Direction.SetXYZ(dir.x(), dir.y(), dir.z());

    G4cout << "[DBG] getting time/energy" << G4endl;
    hit.Time       = trk->GetGlobalTime() / CLHEP::ns;
    hit.Energy     = trk->GetKineticEnergy();   // MeV
    double energy_eV = hit.Energy * 1.0e6;
    hit.Wavelength = (energy_eV > 0.0) ? (1239.84 / energy_eV) : 0.0; // nm

    hit.CopyNo   = -1;
    hit.Detected = false;

    G4cout << "[DBG] calling trk->GetVolume()" << G4endl;
    G4VPhysicalVolume* vol = trk->GetVolume();
    G4cout << "[DBG] vol=" << vol << G4endl;

    if (vol) {
        G4cout << "[DBG] calling vol->GetLogicalVolume()" << G4endl;
        G4LogicalVolume* lv = vol->GetLogicalVolume();
        G4cout << "[DBG] lv=" << lv << G4endl;

        if (lv) {
            G4cout << "[DBG] calling lv->GetSensitiveDetector()" << G4endl;
            G4VSensitiveDetector* sd = lv->GetSensitiveDetector();
            G4cout << "[DBG] sd=" << sd << G4endl;

            if (sd != nullptr) {
                G4cout << "[DBG] calling vol->GetCopyNo()" << G4endl;
                hit.CopyNo   = vol->GetCopyNo();
                hit.Detected = true;
                G4cout << "[DBG] CopyNo=" << hit.CopyNo << G4endl;
            }
        }
    }

    G4cout << "[DBG] calling PhotonManager::AddHit()" << G4endl;
    EDepSim::PhotonManager::Get()->AddHit(hit);
    G4cout << "[DBG] PostUserTrackingAction: done" << G4endl;
}

