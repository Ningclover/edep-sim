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
          Time(0), Energy(0), Wavelength(0), CopyNo(-1) {}
    virtual ~TG4PhotonHit() {}

    /// Absorption point in the sensor volume (mm).
    TVector3 Position;

    /// Photon momentum direction at the moment of absorption (unit vector).
    TVector3 Direction;

    /// Global time of absorption (ns).
    Double_t Time;

    /// Photon energy (MeV).
    Double_t Energy;

    /// Photon wavelength derived from energy: λ = hc/E (nm).
    Double_t Wavelength;

    /// Copy number of the physical sensor volume that recorded the hit (0-based).
    Int_t CopyNo;

    ClassDef(TG4PhotonHit, 1)
};

/// A container of photon hits for one sensitive detector.
typedef std::vector<TG4PhotonHit> TG4PhotonHitContainer;

/// A map from sensitive-detector name to its photon hits.
typedef std::map<std::string, TG4PhotonHitContainer> TG4PhotonHitDetectors;

#endif
