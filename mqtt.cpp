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
  _connected = false;
  _connectHandle = NULL;
}

void MQTT::init(char * theIP, char * theID) {
  if (_connected) {
    disconnect();
  }
  ip = theIP;
  id = theID;
  _mqttUpdate = millis();
}

void MQTT::update() {
  if (not _connected) return;


  // MQTT stuff, check connection status, on disconnect, try reconnect
  if ((long)(millis() - _mqttUpdate) >= 0) {
    _mqttUpdate += _MQTT_UPDATE_INTERVAL;
    // On long time no update, avoid multiupdate
    if ((long)(millis() - _mqttUpdate) >= 0) _mqttUpdate = millis() + _MQTT_UPDATE_INTERVAL;
    
    if (_connected and !_mqttClient.connected()) {
      disconnect();
      _connected = false;
      if (_connectHandle == NULL) {
        // let it check on second core (not in loop)
        MQTT *obj = this;
        xTaskCreate(_connectMqtt,          /* Task function. */
                    "_connectMqtt",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    (void*) obj,       /* Task input parameter */
                    1,                /* Priority of the task. */
                    &_connectHandle);            /* Task handle. */
      }
      return;
    }
  }

  // MQTT loop
 _mqttClient.loop();
}

bool MQTT::connected() {
  if (_connected and !_mqttClient.connected()) {
    disconnect();
    _connected = false;
    if (_connectHandle == NULL) {
      // let it check on second core (not in loop)
      MQTT *obj = this;
      xTaskCreate(_connectMqtt,          /* Task function. */
                  "_connectMqtt",        /* String with name of task. */
                  10000,            /* Stack size in bytes. */
                  (void*) obj,       /* Task input parameter */
                  1,                /* Priority of the task. */
                  &_connectHandle);            /* Task handle. */
    }
  }
  return _mqttClient.connected();

}

// Wrapper since we made another class out of it
void MQTT::subscribe(const char * topic) {
  _mqttClient.subscribe(topic);
  delay(10);
  _mqttClient.loop();
}

// Wrapper since we made another class out of it
void MQTT::publish(const char * topic, const char * msg) {
  _mqttClient.publish(topic, msg);
}

bool MQTT::connect() {
  // Check if connection request has already been handled 
  // TODO: will not connect if tried once
  if (_connectHandle != NULL) return false;
  // Check if IDs and stuff is set
  if (ip == NULL or id == NULL) return false;

  // Set server and callbacks
  _mqttClient.setServer(ip, DEFAULT_MQTT_PORT);
  _mqttClient.setCallback(onMessage);

  // Try to connect
  _connect();

  // If still not connected, use free rtos task to establish nonblocking connect
  if (not _connected) {
    if (_connectHandle == NULL) {
      // let it check on second core (not in loop)
      MQTT *obj = this;
      xTaskCreate(_connectMqtt,          /* Task function. */
                  "_connectMqtt",        /* String with name of task. */
                  10000,            /* Stack size in bytes. */
                  (void*) obj,       /* Task input parameter */
                  1,                /* Priority of the task. */
                  &_connectHandle);            /* Task handle. */
    }
  }
  return _connected;
}
  
// _________________________________________________________________________
// Decorator function since we cannot use non static member function in freertos' createTask 
void MQTT::_connectMqtt(void * pvParameters) {
  bool mqtt_connected = ((MQTT*)pvParameters)->_connect();
  while(!mqtt_connected) {
    vTaskDelay(_MQTT_UPDATE_INTERVAL);
    mqtt_connected = ((MQTT*)pvParameters)->_connect();
  }
  ((MQTT*)pvParameters)->_connectHandle = NULL;
  // Delete this task
  vTaskDelete(NULL);
}

bool MQTT::_connect() {
  // If already connected
  if (_mqttClient.connected()) return true;
  if (_mqttClient.connect(id)) {
    // Set connected flag of member
    _connected = true;
    // Call callback
    if (onConnect != NULL) onConnect();
    return true;
  }
  return false;
}


bool MQTT::disconnect() {
  _mqttClient.disconnect();

  _connected = false;

  if (onDisconnect != NULL) onDisconnect();

  return true;
}



