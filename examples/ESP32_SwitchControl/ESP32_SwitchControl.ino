// Use case:
// ESP32 multi-switch control over MQTT.
// Register multiple GPIO switch capabilities and accept remote on/off
// commands from AtanStack control topic.

#include <WiFi.h>
#include <Atanstack.h>

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* DEVICE_PID = "ATN-XXXX-XXXX-XXXX";
const char* DEVICE_SECRET = "REPLACE_WITH_DEVICE_SECRET";

AtanstackClient atanstack;

bool syncClock() {
  configTime(0, 0, "pool.ntp.org", "time.cloudflare.com");
  struct tm timeInfo;
  return getLocalTime(&timeInfo, 20000);
}

const uint8_t SWITCH_GPIO_1 = 2;
const uint8_t SWITCH_GPIO_2 = 4;
const uint8_t SWITCH_GPIO_3 = 5;
#if CONFIG_IDF_TARGET_ESP32C3
const uint8_t STATUS_LED_GPIO = 8;
#else
const uint8_t STATUS_LED_GPIO = 13;
#endif

void connectWifi() {
  Serial.print("wifi: connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  bool ledOn = false;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    ledOn = !ledOn;
    digitalWrite(STATUS_LED_GPIO, ledOn ? LOW : HIGH);
    delay(250);
  }
  digitalWrite(STATUS_LED_GPIO, LOW);
  Serial.println();
  Serial.print("wifi: connected, ip=");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("atanstack: boot switch control");

  pinMode(STATUS_LED_GPIO, OUTPUT);
  digitalWrite(STATUS_LED_GPIO, HIGH);

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
