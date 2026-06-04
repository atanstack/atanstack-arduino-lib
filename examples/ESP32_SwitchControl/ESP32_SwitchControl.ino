// Use case:
// ESP32 multi-switch control over MQTT.
// Register multiple GPIO switch capabilities and accept remote on/off
// commands from AtanStack control topic.

#include <WiFi.h>
#include <Atanstack.h>

const char* WIFI_SSID = "UBA_2.4G";
const char* WIFI_PASSWORD = "izhanhebat123";

WiFiClient wifiClient;
AtanstackClient atanstack(wifiClient);

const uint8_t SWITCH_GPIO_1 = 2;
const uint8_t SWITCH_GPIO_2 = 4;
const uint8_t SWITCH_GPIO_3 = 5;

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
  IPAddress brokerIp;
  if (WiFi.hostByName("mqtt.hrzhkm.xyz", brokerIp)) {
    Serial.print("dns: mqtt.hrzhkm.xyz -> ");
    Serial.println(brokerIp);
  } else {
    Serial.println("dns: failed resolving mqtt.hrzhkm.xyz");
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("atanstack: boot switch control");

  connectWifi();

  AtanstackConfig config;
  config.devicePid = "ATN-973Q-KCLL-KBMF";
  config.deviceSecret = "cnovyTvTA1EzymRfdvEoowQ4U8Yna6GXESFRtTN3";

  if (!atanstack.begin(config)) {
    Serial.print("atanstack: begin failed: ");
    Serial.println(atanstack.lastError());
    return;
  }

  if (!atanstack.switchPin(SWITCH_GPIO_1, LOW)) {
    Serial.print("atanstack: switchPin gpio 2 failed: ");
    Serial.println(atanstack.lastError());
    return;
  }

  if (!atanstack.switchPin(SWITCH_GPIO_2, LOW)) {
    Serial.print("atanstack: switchPin gpio 4 failed: ");
    Serial.println(atanstack.lastError());
    return;
  }

  if (!atanstack.switchPin(SWITCH_GPIO_3, LOW)) {
    Serial.print("atanstack: switchPin gpio 5 failed: ");
    Serial.println(atanstack.lastError());
    return;
  }

  if (!atanstack.connect()) {
    Serial.print("atanstack: connect failed: ");
    Serial.println(atanstack.lastError());
    Serial.print("atanstack: mqtt state=");
    Serial.println(atanstack.mqttState());
    return;
  }

  Serial.println("atanstack: ready for multi-switch commands");
}

void loop() {
  atanstack.loop();

  if (!atanstack.connected()) {
    Serial.print("atanstack: waiting reconnect, lastError=");
    Serial.println(atanstack.lastError());
    Serial.print("atanstack: mqtt state=");
    Serial.println(atanstack.mqttState());
    delay(1000);
    return;
  }

  delay(10);
}
