// Use case:
// Baseline ESP32 integration for AtanStack.
// Use this when you want to connect an ESP32 over WiFi and periodically send
// standard sensor data events (data + optional meta) with minimal setup.

#include <WiFi.h>
#include <ArduinoJson.h>
#include <Atanstack.h>

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* DEVICE_PID = "ATN-XXXX-XXXX-XXXX";
const char* DEVICE_SECRET = "REPLACE_WITH_DEVICE_SECRET";

AtanstackClient atanstack;

int countData = 1;

bool syncClock() {
  configTime(0, 0, "pool.ntp.org", "time.cloudflare.com");
  struct tm timeInfo;
  return getLocalTime(&timeInfo, 20000);
}

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

  if (!syncClock()) {
    Serial.println("clock sync failed; TLS connection stopped");
    return;
  }

  if (!atanstack.begin(DEVICE_PID, DEVICE_SECRET)) {
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
  atanstack.event("count").data("count", countData).send();

  float temperature = 23.5;
  atanstack.event("temperature-data")
      .data("temperature_c", temperature)
      .meta("firmware metadata", "0.1.0")
      .send();

  float distance = 30.0;
  atanstack.event("distance-sensor").data("distance", distance).send();

  countData++;
  delay(5000);
}
