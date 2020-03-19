#ifndef STASSID
#define STASSID "ASM-modul"
#define STAPSK  "dreebit001"
#endif

// includes
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ElegantOTA.h>

const char* ap_default_ssid = STASSID; ///< Default SSID.
const char* ap_default_psk = STAPSK; ///< Default PSK.

#define HOSTNAME "ESP8266-OTA-"




ESP8266WebServer server(80);


void setup(void) {
  String station_ssid = "esp001";
  String station_psk = "12345678";
  
  Serial.begin(115200);
  delay(100);

  // Set Hostname.
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);

   // Initialize file system.
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  // Check WiFi connection
  // ... check mode
  if (WiFi.getMode() != WIFI_STA) {
    WiFi.mode(WIFI_STA);
    delay(10);
  }

// ... Compare file config with sdk config.
  if (WiFi.SSID() != station_ssid || WiFi.psk() != station_psk) {
    Serial.println("WiFi config changed.");

    // ... Try to connect to WiFi station.
    WiFi.begin(station_ssid.c_str(), station_psk.c_str());

    // ... Pritn new SSID
    Serial.print("new SSID: ");
    Serial.println(WiFi.SSID());

    // ... Uncomment this for debugging output.
    //WiFi.printDiag(Serial);
  } else {
    // ... Begin with sdk config.
    WiFi.begin();
  }

Serial.println("Wait for WiFi connection.");

  // ... Give ESP 10 seconds to connect to station.
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    Serial.write('.');
    //Serial.print(WiFi.status());
    delay(500);
  }
  Serial.println();

  // Check connection
  if (WiFi.status() == WL_CONNECTED) {
    // ... print IP Address
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Can not connect to WiFi station. Go into AP mode.");

    // Go into software AP mode.
    WiFi.mode(WIFI_AP);

    delay(10);

    WiFi.softAP(ap_default_ssid, ap_default_psk);

    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  }

  

  
  
  server.on("/", []() {
    server.send(200, "text/plain", "Hi! I am ESP8266.");
  });

  ElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
