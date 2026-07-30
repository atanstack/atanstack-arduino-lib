// Use case:
// Baseline ESP8266 integration for AtanStack.
// Use this when you want to connect an ESP8266 over WiFi and publish
// sensor data events with explicit data/meta objects.

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Atanstack.h>

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char ATANSTACK_ROOT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIICCTCCAY6gAwIBAgINAgPlwGjvYxqccpBQUjAKBggqhkjOPQQDAzBHMQswCQYD
VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEUMBIG
A1UEAxMLR1RTIFJvb3QgUjQwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAwMDAw
WjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2Vz
IExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNi
AATzdHOnaItgrkO4NcWBMHtLSZ37wWHO5t5GvWvVYRg1rkDdc/eJkTBa6zzuhXyi
QHY7qca4R9gq55KRanPpsXI5nymfopjTX15YhmUPoYRlBtHci8nHc8iMai/lxKvR
HYqjQjBAMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW
BBSATNbrdP9JNqPV2Py1PsVq8JQdjDAKBggqhkjOPQQDAwNpADBmAjEA6ED/g94D
9J+uHXqnLrmvT/aDHQ4thQEd0dlq7A/Cr8deVl5c1RxYIigL9zC2L7F8AjEA8GE8
p/SgguMh1YQdc4acLa/KNJvxn7kjNuK8YAOdgLOaVsjh4rsUecrNIdSUtUlD
-----END CERTIFICATE-----
)EOF";

BearSSL::WiFiClientSecure wifiClient;
BearSSL::X509List rootCa(ATANSTACK_ROOT_CA);
AtanstackClient atanstack(wifiClient);

bool syncClock() {
  configTime(0, 0, "pool.ntp.org", "time.cloudflare.com");
  struct tm timeInfo;
  return getLocalTime(&timeInfo, 20000);
}

void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void setup() {
  Serial.begin(115200);
  connectWifi();

  wifiClient.setTrustAnchors(&rootCa);
  if (!syncClock()) {
    Serial.println("clock sync failed; TLS connection stopped");
    return;
  }

  AtanstackConfig config;
  config.devicePid = "ATN-XXXX-XXXX-XXXX";
  config.deviceSecret = "REPLACE_WITH_DEVICE_SECRET";

  atanstack.begin(config);
  atanstack.connect();
}

void loop() {
  atanstack.loop();

  atanstack.event("sensor-data")
      .data("humidity_pct", 67.4)
      .meta("relay_on", true)
      .send();
  delay(5000);
}
