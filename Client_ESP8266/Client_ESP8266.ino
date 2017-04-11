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

// Access point credentials
const char *ap_ssid = "ShotClock";
const char *ap_password = "12345678";

String sta_ssid("Controller");
String sta_passwd("12345678");

IPAddress serverIP(192,168,6,1);
uint16_t localPort = 1234;

unsigned long last_connect;
unsigned long interval = 5000L;
bool bServerConnect;
bool bWiFiConnect;

void setup(void) {
  Serial.begin(19200);
  
#ifdef DebugSerial
  DebugSerial.println();
  DebugSerial.println("ShotClock initializing...");
  DebugSerial.print("Connecting to SSID:");
  DebugSerial.println(WiFi.SSID());
#endif  

  IPAddress local_IP(192,168,5,1);
  IPAddress gateway(192,168,5,1);
  IPAddress subnet(255,255,255,0);

  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ap_ssid,ap_password); 
  
  webserver_setup();
}

void loop(void) {
  webserver_loop();
  wifiClient_loop();
  delay(10);
}

WiFiClient wifiClient;
#define _echo_commands_ 0

void wifiClient_loop(void)
{
  unsigned long curr_connect = millis();
  
  if ((WiFi.status()==WL_CONNECTED)&&(wifiClient.connected()))
  {
    last_connect = curr_connect;
    
    while (Serial.available()>0) {
      uint8_t b = Serial.read();
      wifiClient.write(b);
    }
    while (wifiClient.available()>0) {
      uint8_t b = wifiClient.read();
      Serial.write(b);
    #if (_echo_commands_)
      wifiClient.write(b);
    #endif
    }
  }
  else
  {
    if (curr_connect - last_connect >= interval)
    {
      last_connect = curr_connect;
      
      if (WiFi.status()==WL_CONNECTED) 
      {
        if (bWiFiConnect)
        {
          #ifdef DebugSerial
          DebugSerial.print("WLAN Connected to ");
          DebugSerial.println(sta_ssid);
          #endif
          bWiFiConnect = false;
          if (WiFi.gatewayIP()==IPAddress(192,168,6,1))
          {
            serverIP = IPAddress(192,168,6,1);
          }
          if (WiFi.gatewayIP()==serverIP)
          {
            serverConnect(true);
          }
          else
          {
            serverConnect(false);
          }
        }
        else if (bServerConnect)
        {
          wifiClient.stop();
          serverConnect(true);
        }
      }
      else
      {
        interval = 5000L;
        #ifdef DebugSerial
        DebugSerial.write('.');
        #endif
        WiFi.begin(sta_ssid.c_str(), sta_passwd.c_str());
        if (bServerConnect) serverConnect(false);
      }
    }
  }
}

void serverConnect(bool Connect)
{
  if (Connect)
  {
    bServerConnect = true;
    last_connect = millis();
    interval = 2000L;
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
    bServerConnect = false;
    wifiClient.stop();
    #ifdef DebugSerial
    DebugSerial.println("Disconnected from server.");
    #endif
  }
}


