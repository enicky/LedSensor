#ifndef MQTTSERVICE_H
#define MQTTSERVICE_H

#include <PubSubClient.h>
#include <WiFiClient.h>
#include "sensorresult.h"

#define MQTT_TOPIC_HUMIDITY "home/printer/bme280/humidity"
#define MQTT_TOPIC_TEMPERATURE "home/printer/bme280/temperature"
#define MQTT_TOPIC_PRESSURE "home/printer/bme280/pressure"
#define MQTT_TOPIC_STATE "home/printer/bme280/status"
#define MQTT_PUBLISH_DELAY 60000
#define MQTT_CLIENT_ID "envsens-device-01"

class MqttService{
    private:
        WiFiClient client;
        PubSubClient mqttClient;
        void publish(char const *topic, float payload);
    public:
        MqttService(WiFiClient c);
        
        void publish(SensorResult SensorResult);
        void connect(void);
        bool isConnected(void);
};

#endif