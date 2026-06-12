#ifndef ATANSTACK_MQTT_H
#define ATANSTACK_MQTT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <time.h>

struct AtanstackConfig {
  const char* brokerHost;
  uint16_t brokerPort;
  const char* devicePid;
  const char* deviceSecret;
  const char* mqttUsername;
  const char* clientId;
  const char* topicBase;
  size_t maxPayloadBytes;
  unsigned long reconnectIntervalMs;

  AtanstackConfig()
      // : brokerHost("192.168.1.52"),
      : brokerHost("mqtt.hrzhkm.xyz"),
        brokerPort(1883),
        devicePid(""),
        deviceSecret(""),
        mqttUsername(""),
        clientId(""),
        topicBase("atanstack/v1/devices"),
        maxPayloadBytes(512),
        reconnectIntervalMs(5000) {}
};

class AtanstackClient {
 public:
  static const uint8_t low = LOW;
  static const uint8_t high = HIGH;

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
  bool switchPin(uint8_t gpio, uint8_t activeLevel = low);
  const char* lastError() const;
  int mqttState();

 private:
  static const uint8_t kSlotCount = 8;
  struct SwitchSlot {
    uint8_t gpio;
    uint8_t activeLevel;
    bool used;
  };

  PubSubClient _mqtt;
  static AtanstackClient* _activeInstance;
  AtanstackConfig _config;
  unsigned long _lastReconnectAttemptMs;
  bool _ntpInitAttempted;
  unsigned long _lastNtpAttemptMs;
  String _lastError;
  DynamicJsonDocument* _dataSlots[kSlotCount];
  DynamicJsonDocument* _metaSlots[kSlotCount];
  SwitchSlot _switchSlots[kSlotCount];
  uint8_t _nextDataSlot;
  uint8_t _nextMetaSlot;

  static void onMqttMessage(char* topic, byte* payload, unsigned int length);
  bool subscribeControlTopic();
  bool buildControlTopic(String& outTopic) const;
  bool buildPingTopic(String& outTopic) const;
  bool buildPongTopic(String& outTopic) const;
  bool publishPong(const char* requestId);
  bool publishSwitchCapability(uint8_t slotIndex, uint8_t gpio, uint8_t activeLevel);
  bool findSwitchSlotByGpio(uint8_t gpio, uint8_t& outIndex) const;
  bool isSwitchStateOn(const JsonDocument& doc, bool& outOn) const;
  bool applySwitchState(uint8_t gpio, bool on);
  void handleMqttMessage(char* topic, byte* payload, unsigned int length);
  bool validateConfig(const AtanstackConfig& config);
  bool validateSendArgs(const char* eventType, JsonObject data);
  bool validateKey(const char* key);
  JsonObject nextDataObject();
  JsonObject nextMetaObject();
  bool buildTopic(const char* eventType, String& outTopic) const;
  void buildTimestamp(String& outTimestamp);
  void ensureClockSynced();
  void setError(const char* message);
};

#endif
