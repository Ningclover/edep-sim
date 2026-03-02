#ifndef EDepSim_PhotonSD_h
#define EDepSim_PhotonSD_h 1

/// A sensitive detector that records individual optical photon hits.
///
/// Each hit stores the photon's arrival position, time, wavelength, and
/// the momentum direction at the moment of absorption.  The collection is
/// written to the ROOT output as a separate branch "PhotonDetector" alongside
/// the usual SegmentDetectors.
///
/// To use: tag a logical volume in the GDML with
///   <auxiliary auxtype="SensDet" auxvalue="PhotonDetector"/>
/// and make sure the SDFactory uses type "photon" for that SD name.

#include "G4VSensitiveDetector.hh"
#include "G4THitsCollection.hh"
#include "G4VHit.hh"
#include "G4ThreeVector.hh"

// ---------------------------------------------------------------------------
// Hit class – one entry per absorbed optical photon
// ---------------------------------------------------------------------------
namespace EDepSim { class PhotonHit; }

class EDepSim::PhotonHit : public G4VHit {
public:
    PhotonHit() {}
    virtual ~PhotonHit() {}

    G4ThreeVector position;     // absorption point (mm)
    G4ThreeVector direction;    // photon momentum direction at absorption
    G4double      time;         // global time of absorption (ns)
    G4double      energy;       // photon energy (MeV)
    G4double      wavelength;   // wavelength (nm), derived from energy
    G4int         copyNo;       // copy number of the physical sensor that was hit
};

// ---------------------------------------------------------------------------
// SD class
// ---------------------------------------------------------------------------
namespace EDepSim { class PhotonSD; }

class EDepSim::PhotonSD : public G4VSensitiveDetector {
public:
    typedef G4THitsCollection<EDepSim::PhotonHit> PhotonHitCollection;

    explicit PhotonSD(G4String name);
    virtual ~PhotonSD();

    void   Initialize(G4HCofThisEvent*) override;
    G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;
    void   EndOfEvent(G4HCofThisEvent*) override;

    /// Access the hit collection after the event.
    PhotonHitCollection* GetHits() const { return fHits; }

private:
    PhotonHitCollection* fHits;
    int                  fHCID;
};

#endif
