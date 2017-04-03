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

IPAddress serverIP(192,168,5,1);
uint16_t localPort = 1234;

bool bConnect;
unsigned long last_connect;

void setup(void) {
  Serial.begin(115200);

  webserver_setup();

  bConnect = false;
  last_connect = millis();
}

void loop(void) {
  webserver_loop();
  wifiClient_loop();
  delay(10);
}

WiFiClient wifiClient;
#define _echo_commands_ 1

void wifiClient_loop(void)
{
  unsigned long curr_connect = millis();
  
  if (WiFi.status()!=WL_CONNECTED) return;  
  
  if (wifiClient.connected())
  {
    while (Serial.available()>0) {
      uint8_t b = Serial.read();
#if (_echo_commands_==0)
      wifiClient.write(b);
#endif    
    }
    while (wifiClient.available()>0) {
      uint8_t b = wifiClient.read();
      Serial.write(b);
#if (_echo_commands_)
      wifiClient.write(b);
#endif
    }
    last_connect = curr_connect;
  }
  else
  {
    if (curr_connect - last_connect >= 1000L)
    {
      if (bConnect)
      {
        wifiClient.stop();
        serverConnect(true);
      }
      last_connect = curr_connect;
    }
  }
}

void serverConnect(bool Connect)
{
  if (Connect)
  {
    bConnect = true;
    last_connect = millis();
    wifiClient.connect(serverIP, localPort);
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
    wifiClient.stop();
    #ifdef DebugSerial
    DebugSerial.print("Disconnected from server.");
    #endif
  }
}


