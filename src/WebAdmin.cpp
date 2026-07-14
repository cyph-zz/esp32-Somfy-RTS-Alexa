#include "WebAdmin.h"
#include <WebServer.h>

namespace {
// Port 80 est réservé à fauxmoESP (requis pour la compatibilité Echo "gen3").
WebServer server(8080);
SomfyRemote* remotePtr = nullptr;

String buildPage() {
    String page;
    page.reserve(1400);
    page += "<!DOCTYPE html><html lang='fr'><head><meta charset='utf-8'>";
    page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    page += "<title>Portail Somfy</title><style>";
    page += "body{font-family:sans-serif;max-width:420px;margin:40px auto;padding:0 16px;background:#f4f4f4;color:#222}";
    page += "h1{font-size:20px}";
    page += ".card{background:#fff;border-radius:8px;padding:16px;margin-bottom:16px;box-shadow:0 1px 3px rgba(0,0,0,.1)}";
    page += ".row{display:flex;justify-content:space-between;padding:4px 0;font-size:14px}";
    page += ".row span:first-child{color:#666}";
    page += ".btn{display:block;text-align:center;padding:14px;margin:8px 0;border-radius:8px;text-decoration:none;font-weight:bold;font-size:16px;color:#fff}";
    page += ".up{background:#2e7d32}.down{background:#1565c0}.stop{background:#ef6c00}.prog{background:#c62828}";
    page += "</style></head><body>";
    page += "<h1>Portail Somfy</h1><div class='card'>";
    page += "<div class='row'><span>Adresse</span><span>0x";
    page += String(remotePtr->address(), HEX);
    page += "</span></div><div class='row'><span>Rolling code</span><span>";
    page += String(remotePtr->rollingCode());
    page += "</span></div></div>";
    page += "<a class='btn up' href='/cmd?b=u'>Ouvrir (U)</a>";
    page += "<a class='btn down' href='/cmd?b=d'>Fermer (D)</a>";
    page += "<a class='btn stop' href='/cmd?b=s'>Stop / My (S)</a>";
    page += "<a class='btn prog' href='/cmd?b=p' onclick=\"return confirm('Envoyer la commande PROG ?')\">Programmer (P)</a>";
    page += "</body></html>";
    return page;
}

void handleRoot() {
    server.send(200, "text/html", buildPage());
}

void handleCmd() {
    if (!server.hasArg("b")) {
        server.send(400, "text/plain", "missing b");
        return;
    }
    String b = server.arg("b");
    if (b == "u") {
        remotePtr->sendCommand(SomfyButton::Up);
    } else if (b == "d") {
        remotePtr->sendCommand(SomfyButton::Down);
    } else if (b == "s") {
        remotePtr->sendCommand(SomfyButton::My);
    } else if (b == "p") {
        remotePtr->sendCommand(SomfyButton::Prog);
    } else {
        server.send(400, "text/plain", "unknown command");
        return;
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleNotFound() {
    server.send(404, "text/plain", "not found");
}
}

void webAdminBegin(SomfyRemote& remote) {
    remotePtr = &remote;
    server.on("/", HTTP_GET, handleRoot);
    server.on("/cmd", HTTP_GET, handleCmd);
    server.onNotFound(handleNotFound);
    server.begin();
}

void webAdminHandle() {
    server.handleClient();
}
