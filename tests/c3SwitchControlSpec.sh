#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SKETCH_PATH="$ROOT_DIR/examples/ESP32_SwitchControl/ESP32_SwitchControl.ino"
PUMP_SKETCH_PATH="$ROOT_DIR/examples/ESP32C3_PumpControl/ESP32C3_PumpControl.ino"

rg -Fq 'WiFi.mode(WIFI_STA);' "$SKETCH_PATH"
rg -Fq 'WiFi.setTxPower(WIFI_POWER_8_5dBm);' "$SKETCH_PATH"
rg -Fq '#if CONFIG_IDF_TARGET_ESP32C3' "$SKETCH_PATH"
rg -Fq 'const uint8_t STATUS_LED_GPIO = 8;' "$SKETCH_PATH"
rg -Fq 'const uint8_t STATUS_LED_GPIO = 13;' "$SKETCH_PATH"
rg -Fq '#endif' "$SKETCH_PATH"
rg -Fq 'pinMode(STATUS_LED_GPIO, OUTPUT);' "$SKETCH_PATH"
rg -Fq 'digitalWrite(STATUS_LED_GPIO, LOW);' "$SKETCH_PATH"
rg -Fq 'digitalWrite(STATUS_LED_GPIO, HIGH);' "$SKETCH_PATH"

PUMP_OFF_LINE="$(rg -n -F 'digitalWrite(PUMP_PIN, RELAY_OFF);' "$PUMP_SKETCH_PATH" | cut -d: -f1)"
PUMP_OUTPUT_LINE="$(rg -n -F 'pinMode(PUMP_PIN, OUTPUT);' "$PUMP_SKETCH_PATH" | cut -d: -f1)"
SERIAL_LINE="$(rg -n -F 'Serial.begin(115200);' "$PUMP_SKETCH_PATH" | cut -d: -f1)"
WIFI_LINE="$(rg -n -F 'connectWifi();' "$PUMP_SKETCH_PATH" | tail -1 | cut -d: -f1)"

test "$PUMP_OFF_LINE" -lt "$PUMP_OUTPUT_LINE"
test "$PUMP_OUTPUT_LINE" -lt "$SERIAL_LINE"
test "$PUMP_OUTPUT_LINE" -lt "$WIFI_LINE"

echo "C3 switch control Wi-Fi configuration spec passed"
