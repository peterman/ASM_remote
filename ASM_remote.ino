#ifndef STASSID
#define STASSID "ASM-modul"
#define STAPSK  "dreebit001"
#endif

// includes
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ElegantOTA.h>
#include <FS.h>
#include <LittleFS.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include "RemoteDebug.h" 

FS* filesystem = &SPIFFS;

const char* ap_default_ssid = STASSID; ///< Default SSID.
const char* ap_default_psk = STAPSK; ///< Default PSK.
String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
String LS_Status="";
String LS_Fil="";
String LS_Rate="";
String LS_Pe="";
unsigned long startTime;

#define HOSTNAME "ESP8266-"

ESP8266WebServer server(80);
//holds the current upload
File fsUploadFile;
RemoteDebug Debug;

#include "filesystem.h"
#include "webserver.h"
#include "serial.h"



void setup(void) {
  String station_ssid = "esp001";
  String station_psk = "12345678";
  
  Serial.begin(9600);
  Serial1.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  delay(100);

  // Set Hostname.
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  String hostNameDebug(HOSTNAME);
  hostNameDebug.concat("debug");
  
  WiFi.hostname(hostname);

  filesystem->begin();
    {
        Dir dir = filesystem->openDir("/");
        while (dir.next()) {
            String fileName = dir.fileName();
            size_t fileSize = dir.fileSize();
        }
    }
    //Load Config -------------------------------------------
    if (!loadConfig()) {
        saveConfig();
    }
    
    

  // Check WiFi connection
  // ... check mode
  if (WiFi.getMode() != WIFI_STA) {
    WiFi.mode(WIFI_STA);
    delay(10);
  }

// ... Compare file config with sdk config.
  if (WiFi.SSID() != station_ssid || WiFi.psk() != station_psk) {
    
    // ... Try to connect to WiFi station.
    WiFi.begin(station_ssid.c_str(), station_psk.c_str());

    // ... Pritn new SSID
    
    // ... Uncomment this for debugging output.
    //WiFi.printDiag(Serial1);
  } else {
    // ... Begin with sdk config.
    WiFi.begin();
  }


  // ... Give ESP 10 seconds to connect to station.
  startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
  }

  // Check connection
  if (WiFi.status() != WL_CONNECTED) {
    // Go into software AP mode.
    WiFi.mode(WIFI_AP);

    delay(10);

    WiFi.softAP(ap_default_ssid, ap_default_psk);

    }

  

  
  
//  server.on("/", []() {server.send(200, "text/plain", "Hi! I am ESP8266.");  });
  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, []() {
      if (!handleFileRead("/edit.htm")) {
          server.send(404, "text/plain", "FileNotFound");
      }
     });
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
      server.send(200, "text/plain", "");
      }, handleFileUpload);
  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
      if (!handleFileRead(server.uri())) {
          server.send(404, "text/plain", "FileNotFound");
      }
    });

  //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/all", HTTP_GET, []() {
    String json = "{";
    json += "\"LR\":" + LS_Rate;
    json += ", \"ST\":" + LS_Status;
    json += ", \"PE\":" + LS_Pe;
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });
  

  MDNS.begin(hostNameDebug);
  MDNS.addService("telnet", "tcp", 23);
  // Initialize RemoteDebug

  Debug.begin(hostNameDebug); // Initialize the WiFi server

  Debug.setResetCmdEnabled(true); // Enable the reset command
  Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)
  Debug.showColors(true); // Colors

  ElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial1.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  Debug.handle();
  ESPserialEvent();
  if (millis() - startTime > 5000) {
    debugV("%d", millis());
    debugV("%s",inputString.c_str());
    startTime = millis();
    }
  if (stringComplete) {
    
    LS_Status=inputString.substring(0,15);  debugI("Status:       %s", LS_Status.c_str());
    LS_Fil=inputString.substring(15,18);    debugI("Filament:     %s", LS_Fil.c_str());
    LS_Rate=inputString.substring(21,29);   debugI("Leckrate:     %s", LS_Rate.c_str());
    LS_Pe=inputString.substring(32,40);     debugI("Einlassdruck: %s", LS_Pe.c_str()); 
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
}
