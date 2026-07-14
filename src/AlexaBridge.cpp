#include "AlexaBridge.h"
#include <fauxmoESP.h>

namespace {
fauxmoESP fauxmo;
SomfyRemote* remotePtr = nullptr;
}

void alexaBegin(SomfyRemote& remote) {
    remotePtr = &remote;

    fauxmo.createServer(true);
    fauxmo.setPort(80); // requis pour la compatibilité Echo "gen3"
    fauxmo.addDevice("portail");

    fauxmo.onSetState([](unsigned char device_id, const char* device_name, bool state, unsigned char value) {
        (void)device_id;
        (void)device_name;
        (void)value;
        if (state) {
            remotePtr->sendCommand(SomfyButton::Up);
        } else {
            remotePtr->sendCommand(SomfyButton::Down);
        }
    });

    fauxmo.enable(true);
}

void alexaHandle() {
    fauxmo.handle();
}
