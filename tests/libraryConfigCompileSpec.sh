#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

arduino-cli compile \
  --fqbn esp32:esp32:esp32c3 \
  --board-options CDCOnBoot=cdc \
  --libraries "$ROOT_DIR" \
  "$ROOT_DIR/examples/ESP32_BasicSend"

arduino-cli compile \
  --fqbn esp32:esp32:esp32c3 \
  --board-options CDCOnBoot=cdc \
  --libraries "$ROOT_DIR" \
  "$ROOT_DIR/examples/ESP32_SwitchControl"

arduino-cli compile \
  --fqbn esp32:esp32:esp32c3 \
  --board-options CDCOnBoot=cdc \
  --libraries "$ROOT_DIR" \
  "$ROOT_DIR/examples/ESP32C3_PumpControl"

arduino-cli compile \
  --fqbn esp32:esp32:esp32c3 \
  --board-options CDCOnBoot=cdc \
  --libraries "$ROOT_DIR" \
  "$ROOT_DIR/examples/esp32CloudSend"

echo "AtanstackConfig compile spec passed"
