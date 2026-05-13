#ifndef ATANSTACK_MQTT_H
#define ATANSTACK_MQTT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

struct AtanstackConfig {
  const char* brokerHost;
  uint16_t brokerPort;
  const char* devicePid;
  const char* clientId;
  const char* topicBase;
  size_t maxPayloadBytes;
  unsigned long reconnectIntervalMs;

  AtanstackConfig()
      : brokerHost("broker.atanstack.com"),
        brokerPort(1883),
        devicePid(""),
        clientId(""),
        topicBase("atanstack/v1/devices"),
        maxPayloadBytes(512),
        reconnectIntervalMs(5000) {}
};

class AtanstackClient {
 public:
  explicit AtanstackClient(Client& networkClient);

  bool begin(const AtanstackConfig& config);
  bool connect();
  void loop();
  bool connected();

  JsonObject data(const char* key, const char* value);
  JsonObject data(const char* key, const String& value);
  JsonObject data(const char* key, int value);
  JsonObject data(const char* key, long value);
  JsonObject data(const char* key, float value);
  JsonObject data(const char* key, double value);
  JsonObject data(const char* key, bool value);

  JsonObject meta(const char* key, const char* value);
  JsonObject meta(const char* key, const String& value);
  JsonObject meta(const char* key, int value);
  JsonObject meta(const char* key, long value);
  JsonObject meta(const char* key, float value);
  JsonObject meta(const char* key, double value);
  JsonObject meta(const char* key, bool value);

  bool send(const char* eventType, JsonObject data, JsonObject meta = JsonObject());
  const char* lastError() const;

 private:
  static const uint8_t kSlotCount = 8;

  PubSubClient _mqtt;
  AtanstackConfig _config;
  unsigned long _lastReconnectAttemptMs;
  String _lastError;
  DynamicJsonDocument* _dataSlots[kSlotCount];
  DynamicJsonDocument* _metaSlots[kSlotCount];
  uint8_t _nextDataSlot;
  uint8_t _nextMetaSlot;

  bool validateConfig(const AtanstackConfig& config);
  bool validateSendArgs(const char* eventType, JsonObject data);
  bool validateKey(const char* key);
  JsonObject nextDataObject();
  JsonObject nextMetaObject();
  bool buildTopic(const char* eventType, String& outTopic) const;
  void setError(const char* message);
};

#endif
