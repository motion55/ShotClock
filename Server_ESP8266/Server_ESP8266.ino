/*
*/
#include <Arduino.h>

#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

#ifndef ESPSerial
#define ESPSerial Serial
bool Stop;
#endif
#define DebugSerial Serial
#ifndef LED_ON
#define LED_ON
#endif
#ifndef LED_OFF
#define LED_OFF
#endif
#ifndef _USE_TCP_ 
#define _USE_TCP_ 1
#endif
#ifndef _USE_UDP_ 
#define _USE_UDP_ 0
#endif

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

// Access point credentials
const char *ap_ssid = "Controller";
const char *ap_password = "12345678";

String sta_ssid;
String sta_passwd;

unsigned int localPort = 1234;
WiFiServer wifiServer(localPort);

#define MAX_SRV_CLIENTS 3
WiFiClient wifiClients[MAX_SRV_CLIENTS];

#define WIFI_CONNECT_INTERVAL   5000L
#define MAX_RECONNECT 2

int Reconnect;
unsigned long last_connect;
unsigned long interval = WIFI_CONNECT_INTERVAL;
bool bWiFiConnect;

// A UDP instance to let us send and receive packets over UDP
#define  _UDP_LISTEN_  0

#if _UDP_LISTEN_
WiFiUDP udp;
#define udp1 udp
#define udp2 udp
#else
WiFiUDP udp1;
WiFiUDP udp2;
#endif

void setup(void) {
  IPAddress local_IP(192,168,6,1);
  IPAddress gateway(192,168,6,1);
  IPAddress subnet(255,255,255,0);

  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ap_ssid,ap_password); 

  if (WiFi.SSID().length()>0)
  {
    sta_ssid = WiFi.SSID();
    sta_passwd = WiFi.psk();
  }
#ifdef DebugSerial
  DebugSerial.println();
  DebugSerial.println("Controller initializing...");
  DebugSerial.print("Connecting to SSID:");
  DebugSerial.print(WiFi.SSID());
  DebugSerial.print("-Password:");
  DebugSerial.println(WiFi.psk());
#endif  
  webserver_setup();
  wifiServer.begin();
  wifiServer.setNoDelay(true);
#if _UDP_LISTEN_
  udp.begin(localPort);
  #ifdef DebugSerial
  DebugSerial.print("Starting UDP ");
  DebugSerial.print("Local port: ");
  DebugSerial.println(udp.localPort());
  #endif  
#endif
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
}

void loop(void) {
  webserver_loop();
  delay(5);
  wifiServer_loop();
  delay(5);
}

void wifiServer_loop(void)
{
  uint8_t idx;
  //check if there are any new clients
  if (wifiServer.hasClient())
  {
    for(idx = 0; idx < MAX_SRV_CLIENTS; idx++)
    {
      //find free/disconnected spot
      if (!wifiClients[idx] || !wifiClients[idx].connected())
      {
        if(wifiClients[idx]) wifiClients[idx].stop();
        wifiClients[idx] = wifiServer.available();
        #ifdef DebugSerial
        if (wifiClients[idx])
        {
          DebugSerial.print(idx);
          DebugSerial.print(".Client accepted:");
          DebugSerial.println(wifiClients[idx].remoteIP().toString());
        }
        #endif  
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = wifiServer.available();
    #ifdef DebugSerial
    if (serverClient)
    {
      DebugSerial.print("Client rejected:");
      DebugSerial.println(serverClient.remoteIP().toString());
    }
    DebugSerial.println("!!!");
    #endif  
    serverClient.stop();
  }
  //check clients for data
#if _UDP_LISTEN_
  {
    size_t cb = udp.parsePacket();
    if (cb>0)
    {
      uint8_t packetBuffer[cb];
      udp.read(packetBuffer, cb);
      Serial.write(packetBuffer, cb);
    }
  }
#endif  
#if 1    
  for(uint8_t i = 0; i < MAX_SRV_CLIENTS; i++)
  {
    if (wifiClients[i] && wifiClients[i].connected())
    {
      size_t len = wifiClients[i].available();
      if(len>0)
      {
        uint8_t buf[len];
        //get data from the telnet client and push it to the UART
        wifiClients[i].read(buf, len);
        ESPSerial.write(buf, len);
      }
    }
  }
  //check UART for data
  size_t len = ESPSerial.available();
  if(len>0)
  {
    uint8_t sbuf[len];
    ESPSerial.readBytes(sbuf,len);
    //push UART data to all connected telnet clients
  #if _USE_UDP_
    Send2UDPStr(sbuf, len);
  #endif
  #if _USE_TCP_  
    Send2ClientStr(sbuf, len);
  #endif    
  }
#endif  
  if (bWiFiConnect)
  {
    unsigned long curr_connect = millis();
    if (curr_connect - last_connect >= interval)
    {
      last_connect = curr_connect;
      interval = WIFI_CONNECT_INTERVAL;
      if (WiFi.isConnected())
      {
        #ifdef DebugSerial
        DebugSerial.print("WLAN Connected to ");
        DebugSerial.println(sta_ssid);
        #endif
        bWiFiConnect = false;
      }
      else
      {
        if (Reconnect<MAX_RECONNECT)
        {
          #ifdef DebugSerial
          DebugSerial.print("Reconnect to SSID:");
          DebugSerial.print(sta_ssid);
          DebugSerial.print(" Password:");
          DebugSerial.println(sta_passwd);
          #endif
          Reconnect++;
          interval += WIFI_CONNECT_INTERVAL*Reconnect;
          WiFi.begin(sta_ssid.c_str(), sta_passwd.c_str());
        }
        else
        {
          bWiFiConnect = false;
        }
      }
    }
  }
}

void Send2Clients(uint8_t dat)
{
  for(uint8_t i = 0; i < MAX_SRV_CLIENTS; i++)
  {
    if (wifiClients[i] && wifiClients[i].connected())
    {
      wifiClients[i].write(dat);
      delay(1);
    }
  }
}

void Send2ClientStr(const uint8_t *buf, size_t len)
{
  for(uint8_t i = 0; i < MAX_SRV_CLIENTS; i++)
  {
    if (wifiClients[i] && wifiClients[i].connected())
    {
      wifiClients[i].write(buf, len);
      delay(1);
    }
  }
}

void Send2UDPStr(const uint8_t *buf, size_t len)
{
  if (WiFi.isConnected())
  {
    IPAddress address = WiFi.localIP();
    address[3] = 0xFF;
    udp1.beginPacket(address, localPort);
    udp1.write(buf, len);
    udp1.endPacket();
    delay(1);
    #ifdef DebugSerial
    DebugSerial.write('+');
    #endif
  }

  if (WiFi.softAPgetStationNum()>0)
  {
    IPAddress address = WiFi.softAPIP();
    address[3] = 0xFF;
    udp2.beginPacket(address, localPort);
    udp2.write(buf, len);
    udp2.endPacket();
    delay(1);
    #ifdef DebugSerial
    DebugSerial.write('-');
    #endif
  }
}

