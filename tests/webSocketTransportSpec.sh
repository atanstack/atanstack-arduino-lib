#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
HEADER_PATH="$ROOT_DIR/src/Atanstack.h"
SKETCH_PATH="$ROOT_DIR/examples/esp32CloudSend/esp32CloudSend.ino"
PUMP_SKETCH_PATH="$ROOT_DIR/examples/ESP32C3_PumpControl/ESP32C3_PumpControl.ino"

rg -Fq 'class AtanstackWebSocketClient : public Client' "$HEADER_PATH"
rg -Fq '_webSocket.sendHeader("Sec-WebSocket-Protocol", "mqtt");' "$HEADER_PATH"
rg -Fq 'return status == 101;' "$HEADER_PATH"
rg -Fq 'if (!connected() || size > UINT16_MAX)' "$HEADER_PATH"
rg -Fq 'uint8_t header[4] = {0x82, 0, 0, 0};' "$HEADER_PATH"
rg -Fq 'masked[i] = buffer[offset + i] ^ maskKey' "$HEADER_PATH"
if rg -Fq '_webSocket.begin(_path)' "$HEADER_PATH"; then
  echo "WebSocket handshake omits MQTT subprotocol" >&2
  exit 1
fi
rg -Fq 'const char* MQTT_HOST = "mqtt.atanstack.com";' "$SKETCH_PATH"
rg -Fq 'const uint16_t MQTT_PORT = 443;' "$SKETCH_PATH"
rg -Fq 'const char* MQTT_PATH = "/mqtt";' "$SKETCH_PATH"
rg -Fq 'configTime(0, 0, "pool.ntp.org", "time.cloudflare.com");' "$SKETCH_PATH"
rg -Fq 'AtanstackClient atanstack;' "$SKETCH_PATH"
rg -Fq 'config.webSocketPath = MQTT_PATH;' "$SKETCH_PATH"
rg -Fq 'brokerHost("mqtt.atanstack.com")' "$HEADER_PATH"
rg -Fq 'brokerPort(443)' "$HEADER_PATH"
rg -Fq 'webSocketPath("/mqtt")' "$HEADER_PATH"
rg -Fq '_secureNetworkClient->setCACert(kAtanstackRootCa);' "$ROOT_DIR/src/Atanstack.cpp"

rg -Fq 'AtanstackClient atanstack;' "$PUMP_SKETCH_PATH"
if rg -q 'WiFiClientSecure|MQTT_HOST|MQTT_PORT|MQTT_PATH|config\.(brokerHost|brokerPort|webSocketPath)' "$PUMP_SKETCH_PATH"; then
  echo "pump sketch overrides AtanStack Cloud defaults" >&2
  exit 1
fi

if rg -Fq -- '-----BEGIN CERTIFICATE-----' "$SKETCH_PATH"; then
  echo "TLS certificate leaked into user sketch" >&2
  exit 1
fi

if rg -Fq 'setInsecure' "$ROOT_DIR/src" "$SKETCH_PATH"; then
  echo "insecure TLS configuration found" >&2
  exit 1
fi

echo "secure WebSocket transport spec passed"
