# Atanstack Simplified JSON Builder Design

Date: 2026-05-14  
Status: Approved for planning

## 1. Objective
Reduce Arduino sketch verbosity for payload creation while preserving explicit `eventType` and backend compatibility.

Current usage requires manual `StaticJsonDocument` setup in user sketches. The target is a simpler, device-friendly API that still publishes the same envelope contract expected by `atanstack-api-data`.

## 2. Scope
In scope:
- Keep explicit `eventType` in send calls.
- Add lightweight JSON-builder helpers on `AtanstackClient`.
- Preserve backward compatibility with existing `send(eventType, JsonObject data, JsonObject meta)` usage.
- Support boolean values (`true`/`false`) in data and meta.

Out of scope:
- Removing existing send overloads.
- Changing backend topic or payload schema.
- Reworking MQTT connection/auth behavior.

## 3. API Design
### 3.1 Existing API retained
- `bool send(const char* eventType, JsonObject data, JsonObject meta = JsonObject())`

### 3.2 New simplified helpers
- `JsonObject data(const char* key, const char* value)`
- `JsonObject data(const char* key, const String& value)`
- `JsonObject data(const char* key, int value)`
- `JsonObject data(const char* key, long value)`
- `JsonObject data(const char* key, float value)`
- `JsonObject data(const char* key, double value)`
- `JsonObject data(const char* key, bool value)`

Equivalent `meta(...)` overloads with same type set.

### 3.3 New send overloads
- `bool send(const char* eventType)`
- `bool send(const char* eventType, JsonObject data)`

This enables simple patterns like:

```cpp
const JsonObject temperature = atanstack.data("temperature", 17.8);
atanstack.send("temperature-data", temperature);
```

And multi-field patterns:

```cpp
atanstack.data("temperature", 17.8);
atanstack.data("battery_pct", 82);
atanstack.meta("firmware", "0.1.0");
atanstack.send("sensor-data");
```

## 4. Internal Architecture
Add two internal pending buffers on client instance:
- `_pendingDataDoc`
- `_pendingMetaDoc`

`data(...)` writes into `_pendingDataDoc` and returns its `JsonObject` view.  
`meta(...)` writes into `_pendingMetaDoc` and returns its `JsonObject` view.

`send(eventType)` publishes using pending docs.  
`send(eventType, JsonObject data)` publishes provided data with empty meta.

Existing 3-arg send remains the core envelope publisher and can be reused by new overloads.

## 5. Data Flow
1. User calls `data(key, value)` and/or `meta(key, value)`.
2. Library stores fields in pending docs.
3. User calls one of:
   - `send(eventType)`
   - `send(eventType, dataObject)`
   - `send(eventType, dataObject, metaObject)`
4. Library builds standard envelope:
   - `device_id`
   - `event_type`
   - `timestamp`
   - `schema_version`
   - `data`
   - `meta`
5. Library publishes to topic as before.

## 6. Error Handling and Reset Semantics
- Key validation:
  - null/empty key in `data/meta` sets `lastError` and returns empty object.
- Send validation:
  - missing/empty `eventType` fails as today.
  - missing data for explicit-arg overloads fails as today.

Pending buffer clearing rules:
- `send(eventType)` clears pending docs only when publish succeeds.
- If send fails, pending docs are retained for retry.
- External-object overloads do not mutate caller-owned docs.

## 7. Type Semantics
Values must preserve semantic JSON types:
- `bool` -> JSON boolean
- `int/long/float/double` -> JSON number
- `const char*` / `String` -> JSON string

No implicit stringification of numeric/bool values.

## 8. Backward Compatibility
- Existing sketches using manual `StaticJsonDocument` continue to compile.
- Existing API signatures remain valid.
- New builder API is additive.

## 9. Examples and Documentation
Update examples to prefer simplified style and show explicit `eventType` usage.

Primary example target:

```cpp
const JsonObject temperature = atanstack.data("temperature", 17.8);
atanstack.send("temperature-data", temperature);
```

Also document multi-field + meta pattern.

## 10. Testing Strategy
- Compile checks for ESP32/ESP8266 examples.
- Unit-style behavior checks for:
  - each value overload writes correct JSON type.
  - `bool` serialized as `true/false`.
  - `send(eventType)` success clears pending docs.
  - `send(eventType)` failure keeps pending docs.
  - old 3-arg send path still works.

## 11. Risks and Mitigations
- Added internal state can cause confusion if users forget what is pending.
  - Mitigation: document clear-on-success and retained-on-failure behavior clearly.
- Memory pressure from extra docs on constrained boards.
  - Mitigation: size pending docs from `maxPayloadBytes` and fail early with `lastError` on overflow.

## 12. Implementation Boundary
This work is scoped to API ergonomics only. It does not alter transport, auth contract, topic format, or backend ingest requirements.
