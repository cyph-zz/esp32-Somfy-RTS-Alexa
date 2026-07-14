#include "SomfyRemote.h"
#include <Preferences.h>
#include <string.h>

namespace {
constexpr uint16_t SYMBOL = 640;
constexpr const char* NVS_NAMESPACE = "somfy";
}

SomfyRemote::SomfyRemote(uint32_t address, const char* nvsKey, uint8_t txPin)
    : _address(address & 0xFFFFFF), _rollingCode(1), _txPin(txPin) {
    strncpy(_nvsKey, nvsKey, sizeof(_nvsKey) - 1);
    _nvsKey[sizeof(_nvsKey) - 1] = '\0';
}

void SomfyRemote::begin() {
    pinMode(_txPin, OUTPUT);
    digitalWrite(_txPin, LOW);
    loadRollingCode();
}

void SomfyRemote::loadRollingCode() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);
    _rollingCode = prefs.getUShort(_nvsKey, 1);
    prefs.end();
}

void SomfyRemote::saveRollingCode() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false);
    prefs.putUShort(_nvsKey, _rollingCode);
    prefs.end();
}

void SomfyRemote::buildFrame(uint8_t frame[7], SomfyButton button) {
    frame[0] = 0xA7;                              // clé de chiffrement (fixe)
    frame[1] = static_cast<uint8_t>(button) << 4; // bouton, checksum ajouté ensuite
    frame[2] = _rollingCode >> 8;
    frame[3] = _rollingCode & 0xFF;
    frame[4] = _address >> 16;
    frame[5] = _address >> 8;
    frame[6] = _address;

    uint8_t checksum = 0;
    for (uint8_t i = 0; i < 7; i++) {
        checksum ^= frame[i] ^ (frame[i] >> 4);
    }
    frame[1] |= checksum & 0x0F;

    for (uint8_t i = 1; i < 7; i++) {
        frame[i] ^= frame[i - 1];
    }
}

void SomfyRemote::sendBit(bool one) {
    if (one) {
        digitalWrite(_txPin, LOW);
        delayMicroseconds(SYMBOL);
        digitalWrite(_txPin, HIGH);
        delayMicroseconds(SYMBOL);
    } else {
        digitalWrite(_txPin, HIGH);
        delayMicroseconds(SYMBOL);
        digitalWrite(_txPin, LOW);
        delayMicroseconds(SYMBOL);
    }
}

void SomfyRemote::sendFrame(const uint8_t frame[7], uint8_t syncPulses) {
    if (syncPulses == 2) {
        // Réveil du récepteur, uniquement avant la première trame de la salve.
        digitalWrite(_txPin, HIGH);
        delayMicroseconds(9415);
        digitalWrite(_txPin, LOW);
        delayMicroseconds(89565);
    }

    for (uint8_t i = 0; i < syncPulses; i++) {
        digitalWrite(_txPin, HIGH);
        delayMicroseconds(4 * SYMBOL);
        digitalWrite(_txPin, LOW);
        delayMicroseconds(4 * SYMBOL);
    }

    digitalWrite(_txPin, HIGH);
    delayMicroseconds(4550);
    digitalWrite(_txPin, LOW);
    delayMicroseconds(SYMBOL);

    for (uint8_t i = 0; i < 56; i++) {
        bool bitValue = (frame[i / 8] >> (7 - (i % 8))) & 1;
        sendBit(bitValue);
    }

    digitalWrite(_txPin, LOW);
    delayMicroseconds(30415); // silence inter-trame
}

void SomfyRemote::sendCommand(SomfyButton button, uint8_t repeats) {
    uint8_t frame[7];
    buildFrame(frame, button);

    sendFrame(frame, 2);
    for (uint8_t i = 0; i < repeats; i++) {
        sendFrame(frame, 7);
    }

    _rollingCode++;
    saveRollingCode();
}
