#ifndef TG4PhotonHit_hxx_seen
#define TG4PhotonHit_hxx_seen

#include <TObject.h>
#include <TVector3.h>

#include <map>
#include <string>
#include <vector>

/// A single optical-photon hit recorded by the PhotonSD sensitive detector.
/// Each entry corresponds to one photon absorbed (or detected) by a sensor tile.
class TG4PhotonHit : public TObject {
public:
    TG4PhotonHit()
        : Position(0,0,0), Direction(0,0,0),
          Time(0), Energy(0), Wavelength(0), CopyNo(-1), Detected(false) {}
    virtual ~TG4PhotonHit() {}

    /// Final position of the photon (mm).
    /// If Detected=true, this is the entry point into the sensor volume.
    /// If Detected=false, this is wherever the photon was absorbed or lost.
    TVector3 Position;

    /// Photon momentum direction at the final step (unit vector).
    TVector3 Direction;

    /// Global time at the final step (ns).
    Double_t Time;

    /// Photon energy (MeV).
    Double_t Energy;

    /// Photon wavelength derived from energy: λ = hc/E (nm).
    Double_t Wavelength;

    /// Copy number of the sensor tile (0-based, 0-24 for the 5x5 grid).
    /// -1 if the photon did not reach a sensor (Detected=false).
    Int_t CopyNo;

    /// True if the photon was killed by the PhotonSD sensitive detector,
    /// i.e. it reached and was absorbed by a sensor tile.
    /// False if the photon died elsewhere (bulk absorption, wall, etc.).
    Bool_t Detected;

    ClassDef(TG4PhotonHit, 2)
};

/// A container of photon hits for one sensitive detector.
typedef std::vector<TG4PhotonHit> TG4PhotonHitContainer;

/// A map from sensitive-detector name to its photon hits.
typedef std::map<std::string, TG4PhotonHitContainer> TG4PhotonHitDetectors;

#endif
