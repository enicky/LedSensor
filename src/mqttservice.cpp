#include "mqttservice.h"
#include <WiFiClient.h>

//#define DEBUG

const char *MQTT_SERVER = "mqtt.gitlab.be";
const char *MQTT_USER = NULL;     //"mqttuser"; // NULL for no authentication
const char *MQTT_PASSWORD = NULL; //"mqttpassword"; // NULL for no authentication

MqttService::MqttService(WiFiClient c)
{
    client = c;
    mqttClient = PubSubClient("192.168.0.174", 1883, client);
};

void MqttService::publish(char const *topic, float payload)
{
    #ifdef DEBUG
        Serial.print(topic);
        Serial.print(": ");
        Serial.println(payload);
    #endif
    mqttClient.publish(topic, String(payload).c_str());
};

void MqttService::publish(SensorResult result){
    connect();
    #ifdef DEBUG
        Serial.print("[mqttservice] publish => client status : ");
        Serial.println(client.status());
    #endif
    publish(MQTT_TOPIC_HUMIDITY, result.humidity());
    delay(10);
    publish(MQTT_TOPIC_TEMPERATURE, result.temperature());
    delay(10);
    publish(MQTT_TOPIC_PRESSURE, result.pressure());
    delay(10);
    mqttClient.disconnect();
};

void MqttService::connect(void)
{

    while (!mqttClient.connected())
    {
        #ifdef DEBUG
            Serial.print("[mqttService] Attempting MQTT connection...");
        #endif
        // Attempt to connect
        if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_STATE, 1, true, "disconnected",
                               false))
        {
            #ifdef DEBUG
            Serial.println("[mqttService] connected");
            #endif
            // Once connected, publish an announcement...
            mqttClient.publish(MQTT_TOPIC_STATE, "connected", true);
        }
        else
        {
            Serial.print("[mqttService] failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
};

bool MqttService::isConnected(void){
    #ifdef DEBUG
        Serial.print("[mqttservice] isConnected : ");
        Serial.println( mqttClient.connected());
    #endif
    return mqttClient.connected();
};