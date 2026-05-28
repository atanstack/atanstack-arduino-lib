// Use case:
// ESP32 liveness/health-check handling over MQTT.
// Keep this example focused on ping->pong diagnostics without switch control.

#include <WiFi.h>
#include <Atanstack.h>

const char* WIFI_SSID = "UBA_2.4G";
const char* WIFI_PASSWORD = "izhanhebat123";

WiFiClient wifiClient;
AtanstackClient atanstack(wifiClient);

unsigned long lastDiagPrintMs = 0;
uint32_t lastSeenPingCount = 0;
uint32_t lastSeenPongPublishedCount = 0;
uint32_t lastSeenPongFailCount = 0;

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
  Serial.println("atanstack: boot health check");

  connectWifi();

  AtanstackConfig config;
  config.devicePid = "ATN-973Q-KCLL-KBMF";
  config.deviceSecret = "cnovyTvTA1EzymRfdvEoowQ4U8Yna6GXESFRtTN3";

  if (!atanstack.begin(config)) {
    Serial.print("atanstack: begin failed: ");
    Serial.println(atanstack.lastError());
    return;
  }

  if (!atanstack.connect()) {
    Serial.print("atanstack: connect failed: ");
    Serial.println(atanstack.lastError());
    return;
  }

  Serial.println("atanstack: ready for health check diagnostics");
}

void loop() {
  atanstack.loop();

  if (!atanstack.connected()) {
    Serial.print("atanstack: waiting reconnect, lastError=");
    Serial.println(atanstack.lastError());
    delay(1000);
    return;
  }

  const unsigned long now = millis();
  const uint32_t pingCount = atanstack.pingReceivedCount();
  const uint32_t pongPublishedCount = atanstack.pongPublishedCount();
  const uint32_t pongFailCount = atanstack.pongPublishFailCount();

  if (pingCount != lastSeenPingCount ||
      pongPublishedCount != lastSeenPongPublishedCount ||
      pongFailCount != lastSeenPongFailCount) {
    Serial.print("atanstack: liveness diag ping_received=");
    Serial.print(pingCount);
    Serial.print(" pong_published=");
    Serial.print(pongPublishedCount);
    Serial.print(" pong_failed=");
    Serial.print(pongFailCount);
    Serial.print(" last_ping_request_id=");
    Serial.print(atanstack.lastPingRequestId());
    Serial.print(" last_ping_ms=");
    Serial.println(atanstack.lastPingReceivedMs());

    if (strlen(atanstack.lastError()) > 0) {
      Serial.print("atanstack: lastError=");
      Serial.println(atanstack.lastError());
    }

    lastSeenPingCount = pingCount;
    lastSeenPongPublishedCount = pongPublishedCount;
    lastSeenPongFailCount = pongFailCount;
  }

  if ((now - lastDiagPrintMs) >= 10000) {
    lastDiagPrintMs = now;
    Serial.print("atanstack: heartbeat connected=1 ping_received=");
    Serial.print(pingCount);
    Serial.print(" pong_published=");
    Serial.print(pongPublishedCount);
    Serial.print(" pong_failed=");
    Serial.println(pongFailCount);
  }
}
