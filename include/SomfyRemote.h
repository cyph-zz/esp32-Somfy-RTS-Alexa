#pragma once

#include <Arduino.h>

enum class SomfyButton : uint8_t {
    My   = 0x1,
    Up   = 0x2,
    Down = 0x4,
    Prog = 0x8,
};

class SomfyRemote {
public:
    // address: identifiant 24 bits de cette télécommande virtuelle (doit être unique).
    // nvsKey: clé courte (<=15 caractères) pour stocker le rolling code en NVS.
    // txPin: GPIO relié à GDO0 du CC1101.
    SomfyRemote(uint32_t address, const char* nvsKey, uint8_t txPin);

    void begin();
    void sendCommand(SomfyButton button, uint8_t repeats = 2);

    uint32_t address() const { return _address; }
    uint16_t rollingCode() const { return _rollingCode; }

private:
    uint32_t _address;
    uint16_t _rollingCode;
    uint8_t _txPin;
    char _nvsKey[16];

    void buildFrame(uint8_t frame[7], SomfyButton button);
    void sendFrame(const uint8_t frame[7], uint8_t syncPulses);
    void sendBit(bool one);
    void loadRollingCode();
    void saveRollingCode();
};
