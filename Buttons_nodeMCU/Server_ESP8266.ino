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
#include <WiFiUdp.h>

// Access point credentials
const char *ap_ssid = "Controller";
const char *ap_password = "12345678";

String sta_ssid;
String sta_passwd;

unsigned int localPort = 1234;
WiFiServer wifiServer(localPort);

#define MAX_SRV_CLIENTS 2
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

void Server_setup(void) {
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

void Server_loop(void) {
  webserver_loop();
  delay(5);
  wifiServer_loop();
  delay(5);
}

void wifiServer_loop(void)
{
  uint8_t i;
  //check if there are any new clients
  if (wifiServer.hasClient())
  {
    bool result = false;
    for(i = 0; i < MAX_SRV_CLIENTS; i++)
    {
      //find free/disconnected spot
      if (!wifiClients[i] || !wifiClients[i].connected())
      {
        if(wifiClients[i]) wifiClients[i].stop();
        wifiClients[i] = wifiServer.available();
        result = true;
        break;
      }
    }
    //no free/disconnected spot so reject
    if (!result)
    {
      WiFiClient serverClient = wifiServer.available();
      serverClient.stop();
    }
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
#if 0    
  for(i = 0; i < MAX_SRV_CLIENTS; i++)
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
#endif  
  //check UART for data
  size_t len = ESPSerial.available();
  if(len>0)
  {
    uint8_t sbuf[len];
    ESPSerial.readBytes(sbuf,len);
    //push UART data to all connected telnet clients
    Send2UDPStr(sbuf, len);
#if _USE_TCP_  
    Send2ClientStr(sbuf, len);
#endif    
  }

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

