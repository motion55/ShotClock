/*
*/
#include <Arduino.h>

#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

#define DebugSerial Serial

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

unsigned int localPort = 1234;
WiFiServer  wifiServer(localPort);

void setup(void) {
  Serial.begin(19200);
  webserver_setup();
}

void loop(void) {
  webserver_loop();
  wifiServer_loop();
}

WiFiClient wifiClient;

void wifiServer_loop(void)
{
  if (wifiClient.connected())
  {
    while (Serial.available()>0) {
      uint8_t b = Serial.read();
      wifiClient.write(b);
    }
    while (wifiClient.available()>0) {
      uint8_t b = wifiClient.read();
      Serial.write(b);
    }
  }
  else
  {
    if ( wifiServer.hasClient())
    {
      wifiClient = wifiServer.available();
    }
    else
    {
      wifiClient.stop();
    }
  }
}

