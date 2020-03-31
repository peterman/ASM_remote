//load config
bool loadConfig() {
    File configFile = filesystem->open("/config.json", "r");
    if (!configFile) {
        Serial.println("Failed to open config file");
        return false;
    }

    size_t size = configFile.size();
    if (size > 1024) {
        Serial.println("Config file size is too large");
        return false;
    }

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    // We don't use String here because ArduinoJson library requires the input
    // buffer to be mutable. If you don't use ArduinoJson, you may as well
    // use configFile.readString instead.
    configFile.readBytes(buf.get(), size);

    StaticJsonDocument<200> doc;
    auto error = deserializeJson(doc, buf.get());
    if (error) {
        Serial.println("Failed to parse config file");
        return false;
    }

    const char* serverName = doc["serverName"];
    const char* accessToken = doc["accessToken"];

    // Real world application would store these values in some variables for
    // later use.

    Serial.print("Loaded serverName: ");
    Serial.println(serverName);
    Serial.print("Loaded accessToken: ");
    Serial.println(accessToken);
    return true;
}

//save config or new config
bool saveConfig() {
    StaticJsonDocument<200> doc;
    JsonObject root = doc.to<JsonObject>();
    root["serverName"] = "api.example.com";
    root["accessToken"] = "128du9as8du12eoue8da98h123ueh9h98";
    JsonObject wifi = root.createNestedObject("wifi");
    wifi["SSID"] = "test";
    wifi["PSK"] = "0123456789";
    JsonObject cycle = root.createNestedObject("cycle");
    cycle["timer_loop"] = 1000;
    cycle["timer_max"] = 10000;
    cycle["timer_min"] = 100;
    
    
    File configFile = filesystem->open("/config.json", "w");
    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    serializeJson(root, configFile);
    return true;
}

//format bytes
String formatBytes(size_t bytes) {
    if (bytes < 1024) { return String(bytes) + "B"; }
    else if (bytes < (1024 * 1024)) { return String(bytes / 1024.0) + "KB"; }
    else if (bytes < (1024 * 1024 * 1024)) { return String(bytes / 1024.0 / 1024.0) + "MB"; }
    else { return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB"; }
}
