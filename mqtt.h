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
#define MQTT_UPDATE_INTERVAL 30000

class MQTT {
  public:
    MQTT();

    void init(char * theIP, char * theID);
    bool connect();
    bool disconnect();
    void update();

    void publish(const char * topic, const  char * msg);
    void subscribe(const char * topic);

    bool connected;
    void (*onDisconnect)(void);
    void (*onConnect)(void);
    void (*onMessage)(char*, byte*, unsigned int);

    char * ip;
    char * id;

  private:

    // Freertos callbacks need to be static members
    static void _connectMqtt(void * pvParameters);
    bool _connect();

    const char * STANDARD_ID = "-";
    long _mqttUpdate;

    TaskHandle_t _connectHandle;
    WiFiClient _mqtt_client;
    PubSubClient _mqttClient;
};

#endif
