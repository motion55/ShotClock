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

IPAddress serverIP(192,168,4,123);
uint16_t localPort = 1234;

WiFiClient wificlient;

uint32_t prev_time;

void setup(void) {
  Serial.begin(115200);

  webserver_setup();

  prev_time = millis();
}

void loop(void) {
  webserver_loop();
  wificlient_loop();
}

bool bConnect;
unsigned long last_connect = 0;

void wificlient_loop(void)
{
  unsigned long curr_connect = millis();
  
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
    last_connect = curr_connect;
  }
  else
  {
    if (curr_connect - last_connect >= 1000L)
    {
      if (bConnect)
      {
        wificlient.stop();
        serverConnect(true);
      }
    }
  }
}

void serverConnect(bool Connect)
{
  if (Connect)
  {
    bConnect = true;
    last_connect = millis();
    wificlient.connect(serverIP, localPort);
    #ifdef DebugSerial
    DebugSerial.print("Connecting to ");
    DebugSerial.print(serverIP.toString());
    DebugSerial.print(" Port No.:");
    DebugSerial.println(localPort);
    #endif
  }
  else
  {
    bConnect = false;
    wificlient.stop();
    #ifdef DebugSerial
    DebugSerial.print("Disconnected from server.");
    #endif
  }
}


