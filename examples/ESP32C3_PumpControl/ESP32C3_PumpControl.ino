// Use case:
// ESP32-C3 Super Mini pump control over MQTT.
// Register a single active-low relay (pump) as a remote switch capability
// and accept on/off commands from the AtanStack control topic.
// The built-in LED mirrors the pump state as a local status indicator.

#include <WiFi.h>
#include <Atanstack.h>

const char* WIFI_SSID = "UBA_2.4G";
const char* WIFI_PASSWORD = "izhanhebat123";

WiFiClient wifiClient;
AtanstackClient atanstack(wifiClient);

unsigned long lastHeartbeatMs = 0;

const int PUMP_PIN = 6;
const int RELAY_ON = LOW;   // Active-low relay
const int RELAY_OFF = HIGH;
const int LED_PIN = 8;      // Built-in LED on ESP32-C3 Super Mini
const int LED_ON = LOW;     // Inverted logic on this board
const int LED_OFF = HIGH;

void connectWifi() {
  Serial.print("wifi: connecting to ");
  Serial.println(WIFI_SSID);

  // ESP32-C3 Super Mini specific: its PCB antenna/RF design fails to
  // associate at the default WiFi TX power. Forcing STA mode and lowering
  // TX power to 8.5 dBm is the known-good workaround for these boards.
  // (Classic boards like the DevKit V1 do not need this.)
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  // Bounded wait (~15s). Blink the built-in LED while connecting so the
  // board's status is visible even without a serial monitor (the C3 Super
  // Mini's native USB-Serial/JTAG can be hard to read headlessly).
  const unsigned long deadline = millis() + 15000;
  bool ledState = false;
  while (WiFi.status() != WL_CONNECTED && millis() < deadline) {
    Serial.print(".");
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? LED_ON : LED_OFF);
    delay(250);
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    // Don't hang here. Fall through to loop(), which reports reconnect
    // state and heartbeats; the library also retries the broker there.
    digitalWrite(LED_PIN, LED_OFF);
    Serial.println("wifi: not connected within timeout, continuing to loop");
    return;
  }

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
  Serial.println("atanstack: boot pump control");

  // Built-in LED starts off (mirrors pump idle state).
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);

  connectWifi();

  AtanstackConfig config;
  config.devicePid = "";
  config.deviceSecret = "";

  if (!atanstack.begin(config)) {
    Serial.print("atanstack: begin failed: ");
    Serial.println(atanstack.lastError());
    return;
  }

  // Register the pump as an active-low relay switch capability.
  if (!atanstack.switchPin(PUMP_PIN, RELAY_ON)) {
    Serial.print("atanstack: switchPin pump gpio 6 failed: ");
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

  Serial.println("atanstack: ready for pump commands");
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

  // Mirror the pump output level onto the built-in LED so the board shows
  // pump state locally. The library drives PUMP_PIN in response to remote
  // control commands; we just read it back here.
  const bool pumpOn = (digitalRead(PUMP_PIN) == RELAY_ON);
  digitalWrite(LED_PIN, pumpOn ? LED_ON : LED_OFF);

  // Periodic heartbeat so connection state is observable on the serial
  // monitor even when attached after boot (native USB CDC drops the
  // setup() logs if the host opens the port late).
  const unsigned long now = millis();
  if ((now - lastHeartbeatMs) >= 5000) {
    lastHeartbeatMs = now;
    Serial.print("atanstack: heartbeat connected=1 pump=");
    Serial.println(pumpOn ? "on" : "off");
  }

  delay(10);
}
