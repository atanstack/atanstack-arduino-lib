#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
HEADER_PATH="$ROOT_DIR/src/Atanstack.h"
SKETCH_PATH="$ROOT_DIR/examples/esp32CloudSend/esp32CloudSend.ino"

rg -Fq 'class AtanstackWebSocketClient : public Client' "$HEADER_PATH"
rg -Fq '_webSocket.begin(_path)' "$HEADER_PATH"
rg -Fq '_webSocket.beginMessage(TYPE_BINARY)' "$HEADER_PATH"
rg -Fq 'const char* MQTT_HOST = "mqtt.atanstack.com";' "$SKETCH_PATH"
rg -Fq 'const uint16_t MQTT_PORT = 443;' "$SKETCH_PATH"
rg -Fq 'const char* MQTT_PATH = "/mqtt";' "$SKETCH_PATH"
rg -Fq 'configTime(0, 0, "pool.ntp.org", "time.cloudflare.com");' "$SKETCH_PATH"
rg -Fq 'AtanstackClient atanstack(webSocketClient);' "$SKETCH_PATH"
rg -Fq 'config.webSocketPath = MQTT_PATH;' "$SKETCH_PATH"
rg -Fq '_secureNetworkClient->setCACert(kAtanstackRootCa);' "$ROOT_DIR/src/Atanstack.cpp"

if rg -Fq -- '-----BEGIN CERTIFICATE-----' "$SKETCH_PATH"; then
  echo "TLS certificate leaked into user sketch" >&2
  exit 1
fi

if rg -Fq 'setInsecure' "$ROOT_DIR/src" "$SKETCH_PATH"; then
  echo "insecure TLS configuration found" >&2
  exit 1
fi

echo "secure WebSocket transport spec passed"
