// Use case:
// Baseline ESP8266 integration for AtanStack.
// Use this when you want to connect an ESP8266 over WiFi and publish
// simple microcontroller data events with the default AtanStack topic structure.

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Atanstack.h>

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

WiFiClient wifiClient;
AtanstackClient atanstack(wifiClient);

void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void setup() {
  Serial.begin(115200);
  connectWifi();

  AtanstackConfig config;
  config.devicePid = "dev_esp8266_01";

  atanstack.begin(config);
  atanstack.connect();
}

void loop() {
  atanstack.loop();

  StaticJsonDocument<128> dataDoc;
  dataDoc["humidity_pct"] = 67.4;

  atanstack.send("sensor-data", dataDoc.as<JsonObject>());
  delay(5000);
}
