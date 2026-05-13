# Atanstack

Arduino library for publishing device events from ESP32/ESP8266 to AtanStack data ingest.

## Features

- Simple API:
  - `atanstack.begin(config)`
  - `atanstack.send(eventType)`
  - `atanstack.send(eventType, dataJson, metaJson?)`
- Lightweight JSON building helpers:
  - `atanstack.data(key, value)`
  - `atanstack.meta(key, value)`
- Uses `device_pid` as MQTT username.
- Default topic format:
  - `atanstack/v1/devices/{devicePid}/events/{eventType}`

## Installation

Install dependencies from Arduino Library Manager:

- `PubSubClient`
- `ArduinoJson`

Then add this library as ZIP or through Library Manager once published.

## Basic Usage

```cpp
#include <WiFi.h>
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

  const JsonObject data = atanstack.data("temperature_c", 29.1);
  atanstack.data("battery_pct", 82);
  atanstack.meta("firmware", "0.1.0");
  atanstack.meta("board", "esp32");

  atanstack.send("sensor-data", data);
}
```

## Alternate Send Styles

Use pending buffers and send directly:

```cpp
atanstack.data("temperature_c", 29.1);
atanstack.data("battery_pct", 82);
atanstack.meta("firmware", "0.1.0");
atanstack.send("sensor-data");
```

On successful `send(eventType)`, pending data/meta is cleared.
On failed `send(eventType)`, pending data/meta is retained for retry.

## Supported Value Types

`data()` and `meta()` accept:

- `const char*`
- `String`
- `int`
- `long`
- `float`
- `double`
- `bool`

## Payload Contract

Every send publishes:

```json
{
  "device_id": "dev_12345",
  "event_type": "sensor-data",
  "timestamp": "1970-01-01T00:00:00Z",
  "schema_version": 1,
  "data": {"temperature_c": 29.1, "battery_pct": 82},
  "meta": {"firmware": "0.1.0", "board": "esp32"}
}
```

## Notes

- `timestamp` is currently a placeholder RFC3339 value.
- Call `atanstack.loop()` continuously.
- Check `atanstack.lastError()` for failure reason.

## License

MIT
