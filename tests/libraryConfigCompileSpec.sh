#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

arduino-cli compile \
  --fqbn esp32:esp32:esp32c3 \
  --board-options CDCOnBoot=cdc \
  --libraries "$ROOT_DIR" \
  "$ROOT_DIR/examples/ESP32_SwitchControl"

echo "AtanstackConfig compile spec passed"
