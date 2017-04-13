
#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

ESP8266WebServer webserver(80);

void webserver_setup()
{
  /* Set page handler functions */
  webserver.on("/", rootPageHandler);
  webserver.on("/wlan_config", wlanPageHandler);
  webserver.on("/gpio", gpioPageHandler);
  webserver.onNotFound(handleNotFound);

  webserver.begin();

  wifiServer.begin();
  wifiServer.setNoDelay(true);

  bWiFiConnect = true;
  last_connect = millis();
  Reconnect = 0;
  interval = WIFI_CONNECT_INTERVAL;
}

inline void webserver_loop()
{
  webserver.handleClient();
}

/* Root page for the webserver */
void rootPageHandler()
{
  String response_message = "<html><head><title>Buttons Controller Server</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>Button Controller</center></h1>";
  response_message += "<h2><center>Controller Server</center></h3>";

  if (WiFi.status() == WL_CONNECTED)
  {
    response_message += "<h3><center>WLAN Status: Connected</center></h3>";
  }
  else
  {
    response_message += "<h3><center>WLAN Status: Disconnected</center></h3>";
  }

  response_message += "<h4><center><a href=\"/wlan_config\">Configure WLAN settings</a></center></h4>";
  response_message += "<h4><center><a href=\"/gpio\">Display Logo On/Off</h4></li></center></h4>";
  response_message += "</body></html>";

  webserver.send(200, "text/html", response_message);
}

/* WLAN page allows users to set the WiFi credentials */
void wlanPageHandler()
{
  // Check if there are any GET parameters
  bool bChange = false;
  if (webserver.hasArg("ssid"))
  {
    if (sta_ssid!=webserver.arg("ssid"))
    {
      sta_ssid = webserver.arg("ssid");
      bChange = true;
    }
    if (webserver.hasArg("password"))
    {
      if (sta_passwd!=webserver.arg("password"))
      {
        sta_passwd = webserver.arg("password");
        bChange = true;
      }
    }
    else
    {
      if (sta_passwd.length()>0) 
      {
        sta_passwd = "";
        bChange = true;
      }
    }
    
    if ((!bWiFiConnect&&!WiFi.isConnected())||bChange)
    {
      if (sta_passwd.length()>0)
      {
        WiFi.begin(sta_ssid.c_str(), sta_passwd.c_str());
      }
      else
      {
        WiFi.begin(sta_ssid.c_str());
      }
      #ifdef DebugSerial
      DebugSerial.print("Connecting to SSID:");
      DebugSerial.print(sta_ssid);
      DebugSerial.print(" Password:");
      DebugSerial.println(sta_passwd);
      #endif
      webserver_setup();
    }
  }

  String response_message = "";
  response_message += "<html>";
  if (bWiFiConnect)
  {
    response_message += "<head><title>ShotClock Display Client</title>";
    response_message += "<meta http-equiv=\"refresh\" content=\"10; url=/wlan_config\"></head>";
  }
  else
  {
    response_message += "<head><title>ShotClock Display Client</title></head>";
  }
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>WLAN Settings</center></h1>";

  response_message += "<ul><li><a href=\"/\">Return to main page</a></li></ul>";

  if (WiFi.isConnected())
  {
    response_message += "WLAN Status: Connected<br>";
  }
  else
  {
    response_message += "WLAN Status: Disconnected<br>";
  }
  response_message += "Local Address: "+WiFi.localIP().toString()+"<br>";
  response_message += "  Subnet Mask: "+WiFi.subnetMask().toString()+"<br>";
  response_message += "GW IP Address: "+WiFi.gatewayIP().toString()+"<br><br>";

  response_message += "<p>To connect to a WiFi network, please select a network...</p>";

  // Get number of visible access points
  int ap_count = WiFi.scanNetworks();

  if (ap_count == 0)
  {
    response_message += "No access points found.<br>";
  }
  else
  {
    response_message += "<form method=\"get\">";
    // Show access points
    for (uint8_t ap_idx = 0; ap_idx < ap_count; ap_idx++)
    {
      if (String(WiFi.SSID(ap_idx))==sta_ssid)
      {
        response_message += "<input type=\"radio\" name=\"ssid\" value=\"" + String(WiFi.SSID(ap_idx)) + "\" checked>";
      }
      else
      {
        response_message += "<input type=\"radio\" name=\"ssid\" value=\"" + String(WiFi.SSID(ap_idx)) + "\">";
      }
      response_message += String(WiFi.SSID(ap_idx)) + " (RSSI: " + WiFi.RSSI(ap_idx) + ")";
      (WiFi.encryptionType(ap_idx) == ENC_TYPE_NONE) ? response_message += " " : response_message += "*";
      response_message += "<br><br>";
    }

    response_message += "WiFi password (if required):<br>";
    response_message += "<input type=\"text\" name=\"password\"><br>";
    response_message += "<input type=\"submit\" value=\"Connect\">";
    response_message += "</form>";
  }

  response_message += "</body></html>";

  webserver.send(200, "text/html", response_message);
}

/*///////////////////////////////////////////////////////////////////////////*/

/* GPIO page allows you to control the GPIO pins */
void gpioPageHandler()
{
  // Check if there are any GET parameters
  if (webserver.hasArg("gpio2"))
  {
    String dataStr;
    if (webserver.arg("gpio2") == "1")
    {
      Stop = false;
      LED_ON;
      dataStr = "ZZZ";
    }
    else
    {
      Stop = true;
      LED_OFF;
      dataStr = "XXX";
    }
    Send2UDPStr((const uint8_t *)dataStr.c_str(), dataStr.length());
    Send2ClientStr((const uint8_t *)dataStr.c_str(), dataStr.length());
  }

  if (webserver.hasArg("icount"))
  {
  }
  
  if (webserver.hasArg("reset"))
  {
    if (webserver.arg("reset") == "Reset")
    {
    }
  }
  
  /*//////////////////////////////////////////////////////////////*/ 

  String response_message = "<html><head><title>Buttons Controller Server</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>Control GPIO pins</center></h1>";
  response_message += "<form method=\"get\">";

  response_message += "<a href=\"/\">Return to main page</a><br><br>";

  response_message += "Shot Clock:<br>";

  if (Stop)
  {
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"1\" onclick=\"submit();\">Run<br>";
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"0\" onclick=\"submit();\" checked>Stop<br><br>";
  }
  else
  {
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"1\" onclick=\"submit();\" checked>Run<br>";
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"0\" onclick=\"submit();\">Stop<br><br>";
  }

  //response_message += "Initial Count(secs.)<br>";
  //response_message += "<input type=\"text\" name=\"icount\" value=\""+String(Count_Init/10)+"\"><br>";
  //response_message += "<input type=\"submit\" name=\"reset\" value=\"Reset\"><br>";
  
  response_message += "</form>";
  response_message += "</body></html>";

  webserver.send(200, "text/html", response_message);
}

/* Called if requested page is not found */
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webserver.uri();
  message += "\nMethod: ";
  message += (webserver.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webserver.args();
  message += "\n";

  for (uint8_t i = 0; i < webserver.args(); i++)
  {
    message += " " + webserver.argName(i) + ": " + webserver.arg(i) + "\n";
  }

  webserver.send(404, "text/plain", message);
}

