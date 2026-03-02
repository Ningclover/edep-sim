#include "TG4PhotonHitChangeHandler.hxx"
#include "TEventDisplay.hxx"
#include "TGUIManager.hxx"

#include <TG4Event.h>
#include <TG4PhotonHit.h>
#include <TEventManager.hxx>

#include <TGButton.h>
#include <TEveManager.h>
#include <TEvePointSet.h>

#include <sstream>
#include <iostream>

EDep::TG4PhotonHitChangeHandler::TG4PhotonHitChangeHandler() {
    fPhotonList = new TEveElementList("photonHitList", "Optical Photon Endpoints");
    fPhotonList->SetMainColor(kYellow);
    fPhotonList->SetMainAlpha(1.0);
    gEve->AddElement(fPhotonList);
}

EDep::TG4PhotonHitChangeHandler::~TG4PhotonHitChangeHandler() {}

void EDep::TG4PhotonHitChangeHandler::Apply() {
    fPhotonList->DestroyElements();

    if (!EDep::TEventDisplay::Get().GUI().GetShowPhotonHitsButton()->IsOn()) {
        std::cout << "Photon hits disabled" << std::endl;
        return;
    }

    if (!gEDepSimEvent) return;

    std::cout << "Handle optical photon hits" << std::endl;

    // Two point sets: detected (sensor, yellow) and undetected (lost, blue)
    TEvePointSet* detected   = new TEvePointSet("detected");
    TEvePointSet* undetected = new TEvePointSet("undetected");
    detected->SetTitle("Photons reaching a sensor");
    undetected->SetTitle("Photons lost (wall/bulk absorption)");

    detected->SetMarkerColor(kYellow);
    detected->SetMarkerStyle(4);   // open circle
    detected->SetMarkerSize(0.5);

    undetected->SetMarkerColor(kAzure+7);
    undetected->SetMarkerStyle(4);
    undetected->SetMarkerSize(0.3);

    for (TG4PhotonHitDetectors::iterator det = gEDepSimEvent->PhotonHits.begin();
         det != gEDepSimEvent->PhotonHits.end(); ++det) {

        std::cout << "  PhotonHits[" << det->first << "] : "
                  << det->second.size() << " photons" << std::endl;

        for (TG4PhotonHitContainer::iterator ph = det->second.begin();
             ph != det->second.end(); ++ph) {

            std::ostringstream title;
            title << "photon t=" << std::fixed << std::setprecision(2)
                  << ph->Time << " ns  lambda=" << ph->Wavelength << " nm"
                  << "  sensor=" << ph->CopyNo
                  << "  pos=(" << ph->Position.X() << ","
                  << ph->Position.Y() << "," << ph->Position.Z() << ") mm";

            if (ph->CopyNo >= 0) {
                detected->SetNextPoint(ph->Position.X(),
                                       ph->Position.Y(),
                                       ph->Position.Z());
            } else {
                undetected->SetNextPoint(ph->Position.X(),
                                        ph->Position.Y(),
                                        ph->Position.Z());
            }
        }
    }

    std::cout << "  Detected=" << detected->GetN()
              << "  Undetected=" << undetected->GetN() << std::endl;

    fPhotonList->AddElement(detected);
    fPhotonList->AddElement(undetected);
}
