# Atanstack

Arduino library for publishing MQTT events from ESP32/ESP8266 to Atanstack data ingest.

## Features

- Simple API:
  - `atanstack.begin(config)`
  - `atanstack.send(eventType, dataJson, metaJson?)`
- Uses `device_pid` as MQTT username.
- Default topic format:
  - `atanstack/v1/devices/{devicePid}/events/{eventType}`
- Payload matches `atanstack-api-data` ingest contract.

## Installation

Install dependencies from Arduino Library Manager:

- `PubSubClient`
- `ArduinoJson`

Then add this library as ZIP or through Library Manager once published.

## Basic Usage

```cpp
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Atanstack.h>

WiFiClient wifiClient;
AtanstackClient atanstack(wifiClient);

void setup() {
  AtanstackConfig config;
  config.devicePid = "dev_12345";

  atanstack.begin(config);
  atanstack.connect();
}

void loop() {
  atanstack.loop();

  StaticJsonDocument<128> dataDoc;
  dataDoc["temperature_c"] = 29.1;

  StaticJsonDocument<96> metaDoc;
  metaDoc["firmware"] = "0.1.0";

  atanstack.send("sensor-data", dataDoc.as<JsonObject>(), metaDoc.as<JsonObject>());
}
```

## Payload Contract

Every `send()` publishes:

```json
{
  "device_id": "dev_12345",
  "event_type": "sensor-data",
  "timestamp": "1970-01-01T00:00:00Z",
  "schema_version": 1,
  "data": {"temperature_c": 29.1},
  "meta": {"firmware": "0.1.0"}
}
```

## Notes

- `timestamp` is currently a placeholder RFC3339 value. Integrate NTP/RTC in your sketch for real event time in a future update.
- Call `atanstack.loop()` continuously.
- Check `atanstack.lastError()` for failure reason.

## License

MIT
