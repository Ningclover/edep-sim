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

    TG4PhotonHit hit;
    const G4ThreeVector& pos = trk->GetPosition();
    const G4ThreeVector& dir = trk->GetMomentumDirection();
    hit.Position.SetXYZ(pos.x(), pos.y(), pos.z());
    hit.Direction.SetXYZ(dir.x(), dir.y(), dir.z());
    hit.Time       = trk->GetGlobalTime() / CLHEP::ns;
    hit.Energy     = trk->GetKineticEnergy();   // MeV
    double energy_eV = hit.Energy * 1.0e6;
    hit.Wavelength = (energy_eV > 0.0) ? (1239.84 / energy_eV) : 0.0; // nm

    // CopyNo: -1 means the photon did NOT end in a sensor volume.
    // If it did end in a sensor, the touchable gives the sensor copy number.
    const G4VTouchable* touch = trk->GetTouchable();
    hit.CopyNo = touch ? touch->GetCopyNumber() : -1;

    EDepSim::PhotonManager::Get()->AddHit(hit);
}

