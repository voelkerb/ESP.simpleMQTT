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
  _connectHandle = NULL;
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
  if (not connected) return;

  // MQTT loop
 _mqttClient.loop();

  // MQTT stuff, check connection status, on disconnect, try reconnect
  if ((long)(millis() - _mqttUpdate) >= 0) {
    _mqttUpdate += MQTT_UPDATE_INTERVAL;
    // On long time no update, avoid multiupdate
    if ((long)(millis() - _mqttUpdate) >= 0) _mqttUpdate = millis() + MQTT_UPDATE_INTERVAL;
    
    if (connected and !_mqttClient.connected()) {
      disconnect();
      connected = false;
    }
    if (!connected) {
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
  }
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
  if (_connectHandle != NULL) return false;
  // Check if IDs and stuff is set
  if (ip == NULL or id == NULL) return false;
  
  // if we changed server and were connected
  if (_mqttClient.connected()) _mqttClient.disconnect();

  // Set server and callbacks
  _mqttClient.setServer(ip, DEFAULT_MQTT_PORT);
  _mqttClient.setCallback(onMessage);

  // Try to connect
  _connect();

  // If still not connected, use free rtos task to establish nonblocking connect
  if (not connected) {
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
  return connected;
}
  
// _________________________________________________________________________
// Decorator function since we cannot use non static member function in freertos' createTask 
void MQTT::_connectMqtt(void * pvParameters) {
  bool mqttConnected = ((MQTT*)pvParameters)->_connect();;
  while(!mqttConnected) {
    vTaskDelay(MQTT_UPDATE_INTERVAL);
    mqttConnected = ((MQTT*)pvParameters)->_connect();
  }
  // Delete this task
  vTaskDelete(NULL);
}

bool MQTT::_connect() {
  if (_mqttClient.connect(id)) {
    // Set connected flag of member
    connected = true;
    // Call callback
    if (onConnect != NULL) onConnect();
    return true;
  }
  return false;
}


bool MQTT::disconnect() {
  _mqttClient.disconnect();

  connected = false;

  if (onDisconnect != NULL) onDisconnect();

  return true;
}



