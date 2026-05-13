#include "Atanstack.h"

AtanstackClient::AtanstackClient(Client& networkClient)
    : _mqtt(networkClient),
      _lastReconnectAttemptMs(0),
      _lastError(""),
      _pendingDataDoc(0),
      _pendingMetaDoc(0) {}

bool AtanstackClient::begin(const AtanstackConfig& config) {
  if (!validateConfig(config)) {
    return false;
  }

  _config = config;
  _mqtt.setServer(_config.brokerHost, _config.brokerPort);
  _mqtt.setBufferSize((uint16_t)_config.maxPayloadBytes);
  _pendingDataDoc = DynamicJsonDocument(_config.maxPayloadBytes);
  _pendingMetaDoc = DynamicJsonDocument(_config.maxPayloadBytes / 2);
  clearPending();
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

  _lastError = "";
  return true;
}

void AtanstackClient::loop() {
  _mqtt.loop();
  if (_mqtt.connected()) {
    return;
  }

  unsigned long now = millis();
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
  JsonObject obj = pendingData();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::data(const char* key, const String& value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = pendingData();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::data(const char* key, int value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = pendingData();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::data(const char* key, long value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = pendingData();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::data(const char* key, float value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = pendingData();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::data(const char* key, double value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = pendingData();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::data(const char* key, bool value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = pendingData();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::meta(const char* key, const char* value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = pendingMeta();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::meta(const char* key, const String& value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = pendingMeta();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::meta(const char* key, int value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = pendingMeta();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::meta(const char* key, long value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = pendingMeta();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::meta(const char* key, float value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = pendingMeta();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::meta(const char* key, double value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = pendingMeta();
  obj[key] = value;
  return obj;
}

JsonObject AtanstackClient::meta(const char* key, bool value) {
  if (!validateKey(key)) {
    return JsonObject();
  }
  JsonObject obj = pendingMeta();
  obj[key] = value;
  return obj;
}

bool AtanstackClient::send(const char* eventType) {
  JsonObject dataObj = pendingData();
  JsonObject metaObj = pendingMeta();
  bool ok = send(eventType, dataObj, metaObj);
  if (ok) {
    clearPending();
  }
  return ok;
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
  doc["timestamp"] = "1970-01-01T00:00:00Z";
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

bool AtanstackClient::isPendingDocReady(const DynamicJsonDocument& doc) {
  if (doc.capacity() == 0) {
    setError("client_not_initialized");
    return false;
  }
  return true;
}

void AtanstackClient::clearPending() {
  if (_pendingDataDoc.capacity() > 0) {
    _pendingDataDoc.clear();
    _pendingDataDoc.to<JsonObject>();
  }
  if (_pendingMetaDoc.capacity() > 0) {
    _pendingMetaDoc.clear();
    _pendingMetaDoc.to<JsonObject>();
  }
}

JsonObject AtanstackClient::pendingData() {
  if (!isPendingDocReady(_pendingDataDoc)) {
    return JsonObject();
  }
  return _pendingDataDoc.as<JsonObject>();
}

JsonObject AtanstackClient::pendingMeta() {
  if (!isPendingDocReady(_pendingMetaDoc)) {
    return JsonObject();
  }
  return _pendingMetaDoc.as<JsonObject>();
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

void AtanstackClient::setError(const char* message) {
  _lastError = message == nullptr ? "unknown" : message;
}
