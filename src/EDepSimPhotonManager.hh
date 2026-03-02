#ifndef EDepSim_PhotonManager_hh_seen
#define EDepSim_PhotonManager_hh_seen

/// Singleton that collects every optical-photon endpoint during an event.
///
/// PostUserTrackingAction writes one entry per dying opticalphoton.
/// SummarizePhotonDetectors reads the list at end-of-event.
/// BeginOfEventAction calls Clear() to reset between events.

#include "TG4PhotonHit.h"   // TG4PhotonHitContainer

namespace EDepSim { class PhotonManager; }

class EDepSim::PhotonManager {
public:
    static PhotonManager* Get();

    /// Remove all hits accumulated so far (call at BeginOfEventAction).
    void Clear() { fHits.clear(); }

    /// Add one hit (called from PostUserTrackingAction).
    void AddHit(const TG4PhotonHit& hit) { fHits.push_back(hit); }

    /// Read-only access to the current event's hits.
    const TG4PhotonHitContainer& GetHits() const { return fHits; }

private:
    PhotonManager() {}
    static PhotonManager* fInstance;
    TG4PhotonHitContainer fHits;
};

#endif
