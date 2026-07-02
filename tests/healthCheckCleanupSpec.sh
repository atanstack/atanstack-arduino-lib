#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
HEADER_PATH="$ROOT_DIR/src/Atanstack.h"
SOURCE_PATH="$ROOT_DIR/src/Atanstack.cpp"

assert_absent() {
  local pattern="$1"
  local path="$2"
  if rg -n "$pattern" "$path" >/dev/null; then
    echo "unexpected match for pattern '$pattern' in $path" >&2
    exit 1
  fi
}

assert_present() {
  local pattern="$1"
  local path="$2"
  if ! rg -n "$pattern" "$path" >/dev/null; then
    echo "missing expected pattern '$pattern' in $path" >&2
    exit 1
  fi
}

assert_absent_fixed() {
  local pattern="$1"
  local path="$2"
  if rg -F -n "$pattern" "$path" >/dev/null; then
    echo "unexpected fixed match for pattern '$pattern' in $path" >&2
    exit 1
  fi
}

assert_present_fixed() {
  local pattern="$1"
  local path="$2"
  if ! rg -F -n "$pattern" "$path" >/dev/null; then
    echo "missing expected fixed pattern '$pattern' in $path" >&2
    exit 1
  fi
}

assert_absent 'sendHealthCheck' "$HEADER_PATH"
assert_absent 'pingReceivedCount|pongPublishedCount|pongPublishFailCount|lastPingRequestId|lastPingReceivedMs' "$HEADER_PATH"
assert_absent 'sendHealthCheck' "$SOURCE_PATH"
assert_absent_fixed 'send("health-check"' "$SOURCE_PATH"
assert_absent '_pingReceivedCount|_pongPublishedCount|_pongPublishFailCount|_lastPingRequestId|_lastPingReceivedMs' "$SOURCE_PATH"

assert_present '/commands/ping' "$SOURCE_PATH"
assert_present '/status/pong' "$SOURCE_PATH"
assert_present_fixed 'publishPong(' "$SOURCE_PATH"
assert_present 'subscribe_ping_failed' "$SOURCE_PATH"
assert_present_fixed 'return publishSwitchState(slotIndex, on);' "$SOURCE_PATH"

echo "health check cleanup spec passed"
