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
#include <WiFiClient.h>
#include <PubSubClient.h>

#define DEFAULT_MQTT_PORT 1883
#define MQTT_UPDATE_INTERVAL 10000

class MQTT {
  public:
    MQTT();

    void init(char * theIP, char * theID, void (*cb)(char*, byte*, unsigned int));
    void init();
    bool connect();
    void update();

    void publish(char * topic, char * msg);
    void subscribe(char * topic);

    bool connected;
    void (*onDisconnect)(void);
    void (*onConnect)(void);

    char * ip;
    char * id;

  private:

    void _init();

    void (*_cb)(char*, byte*, unsigned int);

    long _mqttUpdate;

    WiFiClient _mqtt_client;
    PubSubClient _mqttClient;
};

#endif
