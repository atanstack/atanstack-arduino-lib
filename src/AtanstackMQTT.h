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
  bool connected() const;

  bool send(const char* eventType, JsonObject data, JsonObject meta = JsonObject());
  const char* lastError() const;

 private:
  PubSubClient _mqtt;
  AtanstackConfig _config;
  unsigned long _lastReconnectAttemptMs;
  String _lastError;

  bool validateConfig(const AtanstackConfig& config);
  bool validateSendArgs(const char* eventType, JsonObject data);
  bool buildTopic(const char* eventType, String& outTopic) const;
  void setError(const char* message);
};

#endif
