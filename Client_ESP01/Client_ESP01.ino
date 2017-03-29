/*

*/

#include <Arduino.h>

#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

IPAddress serverIP(192,168,4,1);
uint16_t localPort = 1234;

WiFiClient wificlient;

uint32_t prev_time;
extern void (*serverConnect)(bool bConnect);

void setup(void) {
  Serial.begin(115200);

  webserver_setup();
  serverConnect = _serverConnect;

  prev_time = millis();
}

void loop(void) {
  webserver_loop();
  wificlient_loop();
}

void wificlient_loop(void)
{
  if (wificlient.connected())
  {
    while (Serial.available()>0) {
      uint8_t b = Serial.read();
      wificlient.write(b);
    }
    while (wificlient.available()>0) {
      uint8_t b = wificlient.read();
      Serial.write(b);
    }
  }
}

void _serverConnect(bool bConnect)
{
  if (bConnect)
  {
    wificlient.connect(serverIP, localPort);
    delay(500);
  }
  else
  {
    wificlient.stop();
  }
}


