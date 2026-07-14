#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <SmartRC_CC1101.h>
#include "SomfyRemote.h"
#include "WebAdmin.h"
#include "AlexaBridge.h"

// GDO0 du CC1101 câblé sur GPIO27 (voir schéma de câblage).
#define PIN_GDO0 27

// Identifiant 24 bits de cette télécommande virtuelle.
// Change cette valeur si tu ajoutes d'autres télécommandes ESP32 (chacune doit avoir une adresse unique).
#define REMOTE_ADDRESS 0x123456

// Nom et mot de passe du point d'accès affiché tant que le WiFi n'est pas configuré.
#define AP_NAME "ESP32-Portail-Setup"
#define AP_PASSWORD "somfysetup"

// Nom d'hôte mDNS: accessible ensuite via http://portail.local
#define MDNS_HOSTNAME "portail"

SmartRC_CC1101 radio;
SomfyRemote portail(REMOTE_ADDRESS, "portail1", PIN_GDO0);

void setup() {
    Serial.begin(115200);
    delay(200);

    WiFiManager wm;
    wm.setConfigPortalTimeout(180);
    if (!wm.autoConnect(AP_NAME, AP_PASSWORD)) {
        Serial.println("Echec de connexion WiFi, redemarrage...");
        delay(1000);
        ESP.restart();
    }
    Serial.print("WiFi connecte, IP: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin(MDNS_HOSTNAME)) {
        Serial.println("mDNS actif: http://" MDNS_HOSTNAME ".local");
    }

    radio.Init(); // doit être appelé avant getCC1101(): c'est lui qui initialise le bus SPI.

    if (radio.getCC1101()) {
        Serial.println("CC1101 detecte sur le bus SPI.");
    } else {
        Serial.println("CC1101 introuvable, verifie le cablage (VCC/GND/SCK/MISO/MOSI/CSN).");
    }

    radio.setMHZ(433.42);
    radio.setPA(10);
    radio.SetTx();

    portail.begin();
    webAdminBegin(portail);
    alexaBegin(portail);

    Serial.println("Pret. Interface web sur http://" MDNS_HOSTNAME ".local:8080, ou tape 'u'/'d'/'s'/'p' + Entree ici.");
    Serial.println("Alexa: dis \"decouvre les appareils\" puis \"allume/eteins portail\".");
}

void loop() {
    webAdminHandle();
    alexaHandle();

    if (Serial.available()) {
        char c = Serial.read();
        switch (c) {
            case 'u':
                Serial.println("-> UP");
                portail.sendCommand(SomfyButton::Up);
                break;
            case 'd':
                Serial.println("-> DOWN");
                portail.sendCommand(SomfyButton::Down);
                break;
            case 's':
                Serial.println("-> MY/STOP");
                portail.sendCommand(SomfyButton::My);
                break;
            case 'p':
                Serial.println("-> PROG (mets le moteur en mode association avant, ou dans les 2 min qui suivent)");
                portail.sendCommand(SomfyButton::Prog);
                break;
            default:
                break;
        }
    }
}
