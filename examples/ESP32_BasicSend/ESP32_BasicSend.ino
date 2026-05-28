// Use case:
// Baseline ESP32 integration for AtanStack.
// Use this when you want to connect an ESP32 over WiFi and periodically send
// standard sensor data events (data + optional meta) with minimal setup.

#include <WiFi.h>
#include <ArduinoJson.h>
#include <Atanstack.h>

const char* WIFI_SSID = "UBA_2.4G";
const char* WIFI_PASSWORD = "izhanhebat123";

WiFiClient wifiClient;
AtanstackClient atanstack(wifiClient);

int countData = 1;

void connectWifi() {
  Serial.print("wifi: connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("wifi: connected, ip=");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("atanstack: boot");
  connectWifi();

  AtanstackConfig config;
  config.devicePid = "ATN-973Q-KCLL-KBMF";
  config.deviceSecret = "PBuEZjk4mUFRLKmfnH5ZEUEWAARITC4tm8tp8b6b";

  if (!atanstack.begin(config)) {
    Serial.print("atanstack: begin failed: ");
    Serial.println(atanstack.lastError());
    return;
  }
  Serial.println("atanstack: begin ok");

  if (!atanstack.connect()) {
    Serial.print("atanstack: connect failed: ");
    Serial.println(atanstack.lastError());
    return;
  }
  Serial.println("atanstack: mqtt connected");
}

void loop() {
  atanstack.loop();
  if (!atanstack.connected()) {
    Serial.print("atanstack: waiting reconnect, lastError=");
    Serial.println(atanstack.lastError());
    delay(1000);
    return;
  }
  const JsonObject count = atanstack.data("count", countData);
  const JsonObject temperature = atanstack.data("temperature_c", 29.1);
  const JsonObject temperature_meta = atanstack.meta("firmware", "0.1.0");
  const JsonObject distance = atanstack.data("distance", 30.0);

  const bool sendCount = atanstack.send("count", count);
  Serial.print("send count: ");
  Serial.println(sendCount ? "ok" : atanstack.lastError());

  const bool tempSent = atanstack.send("temperature-data", temperature, temperature_meta);
  Serial.print("send temperature-data: ");
  Serial.println(tempSent ? "ok" : atanstack.lastError());

  const bool distanceSent = atanstack.send("distance-sensor", distance);
  Serial.print("send distance-sensor: ");
  Serial.println(distanceSent ? "ok" : atanstack.lastError());

  countData++;
  delay(5000);
}
