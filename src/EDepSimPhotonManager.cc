#include "EDepSimPhotonManager.hh"

EDepSim::PhotonManager* EDepSim::PhotonManager::fInstance = nullptr;

EDepSim::PhotonManager* EDepSim::PhotonManager::Get() {
    if (!fInstance) fInstance = new PhotonManager();
    return fInstance;
}
