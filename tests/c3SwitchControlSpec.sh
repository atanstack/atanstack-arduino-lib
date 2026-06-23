#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SKETCH_PATH="$ROOT_DIR/examples/ESP32_SwitchControl/ESP32_SwitchControl.ino"

rg -Fq 'WiFi.mode(WIFI_STA);' "$SKETCH_PATH"
rg -Fq 'WiFi.setTxPower(WIFI_POWER_8_5dBm);' "$SKETCH_PATH"
rg -Fq 'const uint8_t STATUS_LED_GPIO = 8;' "$SKETCH_PATH"
rg -Fq 'pinMode(STATUS_LED_GPIO, OUTPUT);' "$SKETCH_PATH"
rg -Fq 'digitalWrite(STATUS_LED_GPIO, LOW);' "$SKETCH_PATH"
rg -Fq 'digitalWrite(STATUS_LED_GPIO, HIGH);' "$SKETCH_PATH"

echo "C3 switch control Wi-Fi configuration spec passed"
