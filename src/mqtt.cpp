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
  lastCheck = millis();
#if defined(ESP32)
  _connectHandle = NULL;
#endif
  _autoReconnect = true;
}

void MQTT::init(char * theIP, char * theID) {
  init(theIP, theID, true);
}

void MQTT::init(char * theIP, char * theID, bool reconnect) {
  if (_connected) disconnect(false);
  _autoReconnect = reconnect;
  ip = theIP;
  id = theID;
  _mqttUpdate = millis();
}

void MQTT::update() {
  if (not _connected) {
#if !defined(ESP32) 
// #if !defined(ESP8266) && !defined(ESP32) 
    // Update stuff over mqtt
    if ((long)(millis() - lastCheck) >= 0) {
      lastCheck += _MQTT_UPDATE_INTERVAL;
      // On long time no update, avoid multiupdate
      if ((long)(millis() - lastCheck) >= 0) lastCheck = millis() + _MQTT_UPDATE_INTERVAL;
      _connect();
    }
#endif
    return;
  }
  // Check if still connected
  if (_connected and !_mqttClient.connected()) {
    disconnect(true);
    if (_autoReconnect) _startConnectMqtt();
    return;
  }
  // MQTT loop
 _mqttClient.loop();
}

bool MQTT::connected() {
  if (_connected and !_mqttClient.connected()) {
    disconnect(true);
    if (_autoReconnect) _startConnectMqtt();
  }
  return _mqttClient.connected();
}

// Wrapper since we made another class out of it
void MQTT::subscribe(const char * topic) {
  _mqttClient.subscribe(topic);
  _mqttClient.loop();
}

// Wrapper since we made another class out of it
void MQTT::publish(const char * topic, const char * msg) {
  publish(topic, msg, false);
}
void MQTT::publish(const char * topic, const char * msg, bool retained) {
  _mqttClient.publish(topic, msg, retained);
}


bool MQTT::connect() {
#if defined(ESP32)
  // Check if connection request has already been handled 
  // TODO: will not connect if tried once
  if (_connectHandle != NULL) return false;
#endif
  // Check if IDs and stuff is set
  if (ip == NULL or id == NULL or strcmp(ip, "-") == 0) return false;

  // Set server and callbacks
  _mqttClient.setServer(ip, DEFAULT_MQTT_PORT);
  _mqttClient.setCallback(onMessage);

  // Try to connect once
  _connect();

  // If still not connected, use free rtos task to establish nonblocking connect
  if (not _connected and _autoReconnect) _startConnectMqtt();
  return _connected;
}

// _________________________________________________________________________
void MQTT::_startConnectMqtt() {
#if defined(ESP32)
  // start ree rtos task to establish nonblocking connect
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
// #elif defined(ESP8266)
//   // Use lambda function here to avoid static member function problem
//   _checker.attach(_MQTT_UPDATE_INTERVAL/1000, +[](MQTT* instance) { instance->_connect(); }, this);
//   // TODO: This seems to not work. 
//   // It calls the function and this reference seems to be fine as well, but it simply does not reconnect, 
//   // Maybe it has sth to do with the checker disabling stuff
//   // As of no, we have to relay on actively calling update (yaghh...)
// #endif
}

#if defined(ESP32)
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
  //  Try to immidiately connect once
  if (_mqttClient.connect(id)) {
    // Set connected flag of member
    _connected = true;
#if defined(ESP8266)
    _checker.detach();
#endif
    // Call callback
    if (onConnect != NULL) onConnect();
    return true;
  }
  return false;
}


bool MQTT::disconnect() {
  bool wasConnected = _mqttClient.connected();
  disconnect(wasConnected);
}

bool MQTT::disconnect(bool notify) {
  if (_mqttClient.connected()) _mqttClient.disconnect();
  _connected = false;
  if (notify and onDisconnect != NULL) onDisconnect();
  return true;
}

