#include "Atanstack.h"

AtanstackClient::AtanstackClient(Client& networkClient)
    : _mqtt(networkClient),
      _lastReconnectAttemptMs(0),
      _lastHealthCheckMs(0),
      _ntpInitAttempted(false),
      _lastNtpAttemptMs(0),
      _lastError(""),
      _nextDataSlot(0),
      _nextMetaSlot(0) {
  for (uint8_t i = 0; i < kSlotCount; ++i) {
    _dataSlots[i] = nullptr;
    _metaSlots[i] = nullptr;
  }
}

bool AtanstackClient::begin(const AtanstackConfig& config) {
  if (!validateConfig(config)) {
    return false;
  }

  _config = config;
  _mqtt.setServer(_config.brokerHost, _config.brokerPort);
  _mqtt.setBufferSize((uint16_t)_config.maxPayloadBytes);
  for (uint8_t i = 0; i < kSlotCount; ++i) {
    if (_dataSlots[i] != nullptr) {
      delete _dataSlots[i];
    }
    if (_metaSlots[i] != nullptr) {
      delete _metaSlots[i];
    }
    _dataSlots[i] = new DynamicJsonDocument(_config.maxPayloadBytes);
    _metaSlots[i] = new DynamicJsonDocument(_config.maxPayloadBytes / 2);
    _dataSlots[i]->to<JsonObject>();
    _metaSlots[i]->to<JsonObject>();
  }
  _nextDataSlot = 0;
  _nextMetaSlot = 0;
  _lastHealthCheckMs = 0;
  _ntpInitAttempted = false;
  _lastNtpAttemptMs = 0;
  _lastError = "";
  return true;
}

bool AtanstackClient::connect() {
  if (_config.devicePid == nullptr || strlen(_config.devicePid) == 0) {
    setError("missing_device_pid");
    return false;
  }

  if (_mqtt.connected()) {
    return true;
  }

  const char* clientId = (_config.clientId != nullptr && strlen(_config.clientId) > 0)
                             ? _config.clientId
                             : _config.devicePid;

  if (!_mqtt.connect(clientId, _config.devicePid, nullptr)) {
    setError("mqtt_connect_failed");
    return false;
  }

  _lastHealthCheckMs = 0;
  _lastError = "";
  return true;
}

void AtanstackClient::loop() {
  _mqtt.loop();
  unsigned long now = millis();

  if (_mqtt.connected()) {
    ensureClockSynced();
    if (_config.autoHealthCheckEnabled &&
        (now - _lastHealthCheckMs) >= _config.healthCheckIntervalMs) {
      if (sendHealthCheck()) {
        _lastHealthCheckMs = now;
      }
    }
    return;
  }

  if ((now - _lastReconnectAttemptMs) < _config.reconnectIntervalMs) {
    return;
  }

  _lastReconnectAttemptMs = now;
  connect();
}

bool AtanstackClient::connected() { return _mqtt.connected(); }

JsonObject AtanstackClient::data(const char* key, const char* value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = nextDataObject();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::data(const char* key, const String& value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = nextDataObject();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::data(const char* key, int value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = nextDataObject();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::data(const char* key, long value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = nextDataObject();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::data(const char* key, float value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = nextDataObject();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::data(const char* key, double value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = nextDataObject();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::data(const char* key, bool value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = nextDataObject();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::meta(const char* key, const char* value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = nextMetaObject();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::meta(const char* key, const String& value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = nextMetaObject();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::meta(const char* key, int value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = nextMetaObject();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::meta(const char* key, long value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = nextMetaObject();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::meta(const char* key, float value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = nextMetaObject();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::meta(const char* key, double value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = nextMetaObject();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::meta(const char* key, bool value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = nextMetaObject();
  obj[key] = value;
  return obj;
}

bool AtanstackClient::send(const char* eventType, JsonObject data, JsonObject meta) {
  if (!validateSendArgs(eventType, data)) {
    return false;
  }

  if (!_mqtt.connected()) {
    setError("mqtt_not_connected");
    return false;
  }

  String topic;
  if (!buildTopic(eventType, topic)) {
    return false;
  }

  DynamicJsonDocument doc(_config.maxPayloadBytes);
  doc["device_id"] = _config.devicePid;
  doc["event_type"] = eventType;
  String timestamp;
  buildTimestamp(timestamp);
  doc["timestamp"] = timestamp;
  doc["schema_version"] = 1;

  JsonObject payloadData = doc.createNestedObject("data");
  for (JsonPair kv : data) {
    payloadData[kv.key()] = kv.value();
  }

  JsonObject payloadMeta = doc.createNestedObject("meta");
  if (!meta.isNull()) {
    for (JsonPair kv : meta) {
      payloadMeta[kv.key()] = kv.value();
    }
  }

  String payload;
  size_t written = serializeJson(doc, payload);
  if (written == 0) {
    setError("serialize_failed");
    return false;
  }

  if (payload.length() > _config.maxPayloadBytes) {
    setError("payload_too_large");
    return false;
  }

  if (!_mqtt.publish(topic.c_str(), payload.c_str())) {
    setError("publish_failed");
    return false;
  }

  _lastError = "";
  return true;
}

bool AtanstackClient::sendHealthCheck() {
  JsonObject health = data("health", "ok");
  if (health.isNull()) {
    return false;
  }
  return send("health-check", health);
}

const char* AtanstackClient::lastError() const {
  return _lastError.length() == 0 ? "" : _lastError.c_str();
}

bool AtanstackClient::validateConfig(const AtanstackConfig& config) {
  if (config.brokerHost == nullptr || strlen(config.brokerHost) == 0) {
    setError("missing_broker_host");
    return false;
  }
  if (config.devicePid == nullptr || strlen(config.devicePid) == 0) {
    setError("missing_device_pid");
    return false;
  }
  if (config.topicBase == nullptr || strlen(config.topicBase) == 0) {
    setError("missing_topic_base");
    return false;
  }
  if (config.maxPayloadBytes < 256) {
    setError("payload_limit_too_small");
    return false;
  }
  return true;
}

bool AtanstackClient::validateSendArgs(const char* eventType, JsonObject data) {
  if (eventType == nullptr || strlen(eventType) == 0) {
    setError("missing_event_type");
    return false;
  }
  if (data.isNull()) {
    setError("missing_data");
    return false;
  }
  return true;
}

bool AtanstackClient::validateKey(const char* key) {
  if (key == nullptr || strlen(key) == 0) {
    setError("missing_key");
    return false;
  }
  return true;
}

JsonObject AtanstackClient::nextDataObject() {
  DynamicJsonDocument* slot = _dataSlots[_nextDataSlot];
  if (slot == nullptr || slot->capacity() == 0) {
    setError("client_not_initialized");
    return JsonObject();
  }
  slot->clear();
  JsonObject obj = slot->to<JsonObject>();
  _nextDataSlot = (_nextDataSlot + 1) % kSlotCount;
  return obj;
}

JsonObject AtanstackClient::nextMetaObject() {
  DynamicJsonDocument* slot = _metaSlots[_nextMetaSlot];
  if (slot == nullptr || slot->capacity() == 0) {
    setError("client_not_initialized");
    return JsonObject();
  }
  slot->clear();
  JsonObject obj = slot->to<JsonObject>();
  _nextMetaSlot = (_nextMetaSlot + 1) % kSlotCount;
  return obj;
}

bool AtanstackClient::buildTopic(const char* eventType, String& outTopic) const {
  if (_config.topicBase == nullptr || strlen(_config.topicBase) == 0) {
    return false;
  }
  outTopic.reserve(strlen(_config.topicBase) + strlen(_config.devicePid) + strlen(eventType) + 16);
  outTopic = _config.topicBase;
  outTopic += "/";
  outTopic += _config.devicePid;
  outTopic += "/events/";
  outTopic += eventType;
  return true;
}

void AtanstackClient::buildTimestamp(String& outTimestamp) {
  ensureClockSynced();
  const time_t now = time(nullptr);
  if (now < 946684800) {
    outTimestamp = "1970-01-01T00:00:00Z";
    return;
  }

  struct tm timeInfo;
  gmtime_r(&now, &timeInfo);
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &timeInfo);
  outTimestamp = buffer;
}

void AtanstackClient::ensureClockSynced() {
#if defined(ESP32) || defined(ESP8266)
  const unsigned long nowMs = millis();
  if (_ntpInitAttempted && (nowMs - _lastNtpAttemptMs) < 10000) {
    return;
  }

  if (time(nullptr) >= 946684800) {
    return;
  }

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  _ntpInitAttempted = true;
  _lastNtpAttemptMs = nowMs;
#endif
}

void AtanstackClient::setError(const char* message) {
  _lastError = message == nullptr ? "unknown" : message;
}
