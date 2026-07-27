# Atanstack

Arduino library for publishing device events from ESP32/ESP8266 to AtanStack data ingest.

## Features

- Simple API:
  - `atanstack.begin(devicePid, deviceSecret)`
  - `atanstack.event(eventType).data(key, value).send()`
- Uses per-device MQTT credentials (`devicePid` + `deviceSecret`).
- Default topic format:
  - `atanstack/v1/devices/{devicePid}/events/{eventType}`
- Secure cloud connection through MQTT over WebSocket:
  - `wss://mqtt.atanstack.com/mqtt`

## Installation

Install dependencies from Arduino Library Manager:

- `PubSubClient`
- `ArduinoJson`
- `ArduinoHttpClient`

Then add this library as ZIP or through Library Manager once published.

## AtanStack Cloud

Standard `PubSubClient` only speaks raw MQTT. AtanStack Cloud is exposed
through secure WebSocket. `AtanstackClient` enables that transport when
`webSocketPath` is configured:

```cpp
#include <WiFi.h>
#include <Atanstack.h>

AtanstackClient atanstack;

void setup() {
  // Connect Wi-Fi and synchronize system time before TLS.
  // AtanstackClient configures AtanStack Cloud's trusted CA.

  atanstack.begin("ATN-XXXX-XXXX-XXXX", "REPLACE_WITH_DEVICE_SECRET");
  atanstack.connect();
}

void loop() {
  atanstack.loop();

  atanstack.event("sensor-data")
      .data("temperature_c", 29.1)
      .meta("firmware", "0.1.0")
      .send();
}
```

See `examples/esp32CloudSend` for a complete certificate-validated ESP32
sketch.

## Raw MQTT

For a LAN broker or directly exposed MQTT TLS listener, pass the matching
`WiFiClient` or `WiFiClientSecure` directly to `AtanstackClient` and override
`brokerHost`, `brokerPort`, and `webSocketPath`. Public
`mqtt.atanstack.com` ports `1883` and `8883` are not currently exposed.

## Multiple Event Objects

```cpp
atanstack.event("temperature-data")
    .data("temperature_c", 29.1)
    .meta("firmware", "0.1.0")
    .send();

atanstack.event("distance-sensor")
    .data("distance", 30.0)
    .send();
```

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
  "data": {"temperature_c": 29.1},
  "meta": {"firmware": "0.1.0"}
}
```

## Notes

- Synchronize the device clock before a TLS connection.
- Call `atanstack.loop()` continuously.
- Check `atanstack.lastError()` for failure reason.

## License

MIT
