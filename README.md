# SimpleMQTT
MQTT wrapper so that you do not have to deal with reconnects./
It is handled automatically in the background. Just provide the server, the and connect once. If you are still interested in when connection fails, handy callbacks allow you to add support.

```C++
#include "mqtt.h"
..
MQTT mqtt;

void setup() {
  Serial.begin(115200);
  ...
  // Set mqtt and callbacks
  mqtt.init((char*)MQTT_SERVER, (char*)DEVICE_NAME);
  mqtt.onConnect = &onMQTTConnect;
  mqtt.onDisconnect = &onMQTTDisconnect;
  mqtt.onMessage = &mqttCallback;

  bool success = mqtt.connect();
  if (!success) Serial.printf("Error connecting to MQTT Server: %s\n", mqtt.ip);
}

void loop() {
  mqtt.update();
}

// MQTT connect callback
void onMQTTConnect() {
  Serial.printf("MQTT connected to %s\n", mqtt.ip);
  //  Subscribe to topic
  mqtt.subscribe(subscribeTopic);
}

// MQTT Disconnect callback
void onMQTTDisconnect() {
  Serial.printf("MQTT disconnected from %s\n", mqtt.ip);
}

// MQTT msg callback
void mqttCallback(char* topic, byte* msg, unsigned int length) {
  Serial.printf("MQTT msg on topic: %s:\n", topic);
}

```