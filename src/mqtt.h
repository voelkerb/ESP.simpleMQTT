/***************************************************
 Library for interfacing with a microcontroller over 
 mqtt. Subscribe to differen topics and have callback functions

 Feel free to use the code as it is.

 Benjamin Völker, voelkerb@me.com
 Embedded Systems
 University of Freiburg, Institute of Informatik
 ****************************************************/

#ifndef MQTT_h
#define MQTT_h

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#if defined(ESP32)
#include <WiFi.h>
#include <WiFiClient.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <Ticker.h>
#endif
#include <PubSubClient.h>

#define DEFAULT_MQTT_PORT 1883
// MQTT is important, do not wait too long
#define _MQTT_UPDATE_INTERVAL 1000

class MQTT {
  public:
    MQTT();

    void init(char * theIP, char * theID);
    void init(char * theIP, char * theID, bool reconnect);
    bool connect();
    bool disconnect();
    bool disconnect(bool notify);
    bool connected();
    void update();

    void publish(const char * topic, const  char * msg);
    void publish(const char * topic, const  char * msg, bool retained);
    void subscribe(const char * topic);

    void (*onDisconnect)(void);
    void (*onConnect)(void);
    void (*onMessage)(char*, byte*, unsigned int);

    char * ip;
    char * id;

    bool _connect();
  private:
    bool _connected;
    bool _autoReconnect;

#if defined(ESP32)
    // Freertos callbacks need to be static members
    static void _connectMqtt(void * pvParameters);
    TaskHandle_t _connectHandle;
#elif defined(ESP8266)
    static void _connectMqtt(MQTT* instance);
    Ticker _checker;
#else 
#endif
    void _startConnectMqtt();

    const char * STANDARD_ID = "-";
    uint32_t _mqttUpdate;

    WiFiClient _mqtt_client;
    PubSubClient _mqttClient;
};

#endif