// Use case:
// Baseline ESP32 integration for AtanStack.
// Use this when you want to connect an ESP32 over WiFi and periodically send
// standard sensor data events (data + optional meta) with minimal setup.

#include <WiFi.h>
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
  config.devicePid = "dev_12345";

  atanstack.begin(config);
  atanstack.connect();
}

void loop() {
  atanstack.loop();

  const JsonObject temperature = atanstack.data("temperature_c", 29.1);
  atanstack.data("battery_pct", 82);
  atanstack.meta("firmware", "0.1.0");
  atanstack.meta("board", "esp32");

  atanstack.send("sensor-data", temperature);
  delay(5000);
}
