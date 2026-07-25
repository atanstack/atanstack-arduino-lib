// ESP32 -> AtanStack Cloud using MQTT over secure WebSocket.

#include <WiFi.h>
#include <Atanstack.h>

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* DEVICE_PID = "ATN-XXXX-XXXX-XXXX";
const char* DEVICE_SECRET = "REPLACE_WITH_DEVICE_SECRET";

AtanstackClient atanstack;

unsigned long lastSendMs = 0;

bool syncClock() {
  configTime(0, 0, "pool.ntp.org", "time.cloudflare.com");
  struct tm timeInfo;
  return getLocalTime(&timeInfo, 20000);
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }

  if (!syncClock()) {
    Serial.println("clock sync failed; TLS connection stopped");
    return;
  }

  if (!atanstack.begin(DEVICE_PID, DEVICE_SECRET) || !atanstack.connect()) {
    Serial.println(atanstack.lastError());
  }
}

void loop() {
  atanstack.loop();

  if (!atanstack.connected() || millis() - lastSendMs < 5000) {
    return;
  }

  lastSendMs = millis();
  JsonObject reading = atanstack.data("temperature_c", 26.75);
  reading["humidity_pct"] = 68;
  JsonObject details = atanstack.meta("firmware", "esp32-cloud-1.0.0");
  details["transport"] = "mqtt-wss";

  if (!atanstack.send("environment", reading, details)) {
    Serial.println(atanstack.lastError());
  }
}
