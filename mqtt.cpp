/***************************************************
 Library for interfacing with a microcontroller over 
 mqtt. Subscribe to differen topics and have callback functions

 Feel free to use the code as it is.

 Benjamin Völker, voelkerb@me.com
 Embedded Systems
 University of Freiburg, Institute of Informatik
 ****************************************************/

#include "mqtt.h"


MQTT::MQTT():_mqttClient(_mqtt_client) {
  ip = (char*)STANDARD_ID;
  id = (char*)STANDARD_ID;
  onDisconnect = NULL;
  onConnect = NULL;
  onMessage = NULL;
  connected = false;
}

void MQTT::init(char * theIP, char * theID) {
  if (connected) {
    disconnect();
  }
  ip = theIP;
  id = theID;
  _mqttUpdate = millis();
}

void MQTT::update() {
  // MQTT loop
 _mqttClient.loop();

  // MQTT stuff, check connection status, on disconnect, try reconnect
  // TODO: no clue why, but does not work properly for esp32 (maybe it is the mac side)
  if ((long)(millis() - _mqttUpdate) >= 0) {
    _mqttUpdate += MQTT_UPDATE_INTERVAL;
    // On long time no update, avoid multiupdate
    if ((long)(millis() - _mqttUpdate) >= 0) _mqttUpdate = millis() + MQTT_UPDATE_INTERVAL;
    
    if (connected and !_mqttClient.connected()) {
      disconnect();
      connected = false;
    }
    if (!connected) {
      connect();
    }
  }
}

// Wrapper since we made another class out of it
void MQTT::subscribe(const char * topic) {
  _mqttClient.subscribe(topic);
  _mqttClient.loop();
}

// Wrapper since we made another class out of it
void MQTT::publish(const char * topic, const char * msg) {
  _mqttClient.publish(topic, msg);
  _mqttClient.loop();
}

bool MQTT::connect() {
  if (ip == NULL or id == NULL) return false;

  // if we changed server and were connected
  if (_mqttClient.connected()) _mqttClient.disconnect();

  // Set server
  _mqttClient.setServer(ip, DEFAULT_MQTT_PORT);
  _mqttClient.setCallback(onMessage);

  // Look if connection is successfull and return if not
  if (!_mqttClient.connect(id)) {
    return false;
  } 

  // Subscribe to all the topics
  connected = true;

  if (onConnect != NULL) onConnect();

  return connected;
}

bool MQTT::disconnect() {
  _mqttClient.disconnect();

  connected = false;

  if (onDisconnect != NULL) onDisconnect();

  return true;
}



