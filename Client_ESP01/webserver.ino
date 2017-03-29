
#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

ESP8266WebServer webserver(80);

void (*serverConnect)(bool bConnect) = _DummyConnect;

void webserver_setup()
{
  /* Set page handler functions */
  webserver.on("/", rootPageHandler);
  webserver.on("/wlan_config", wlanPageHandler);
  webserver.on("/gpio", serverPageHandler);
  webserver.onNotFound(handleNotFound);

  webserver.begin();
}

inline void webserver_loop()
{
  webserver.handleClient();
}

/* Root page for the webserver */
void rootPageHandler()
{
  String response_message = "<html><head><title>ESP8266 Webserver</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>ESP8266 Webserver</center></h1>";

  if (WiFi.status() == WL_CONNECTED)
  {
    response_message += "<h3><center>WLAN Status: Connected</center></h3>";
  }
  else
  {
    response_message += "<h3><center>WLAN Status: Disconnected</center></h3>";
  }

  response_message += "<h4><center><a href=\"/wlan_config\">Configure WLAN settings</a></center></h4>";
  response_message += "<h4><center><a href=\"/gpio\">Configure Server</h4></li></center></h4>";
  response_message += "</body></html>";

  webserver.send(200, "text/html", response_message);
}

/* WLAN page allows users to set the WiFi credentials */
void wlanPageHandler()
{
  // Check if there are any GET parameters
  if (webserver.hasArg("ssid"))
  {
    if (webserver.hasArg("password"))
    {
      WiFi.begin(webserver.arg("ssid").c_str(), webserver.arg("password").c_str());
    }
    else
    {
      WiFi.begin(webserver.arg("ssid").c_str());
    }

    for (int i = 0; i<200; i++)
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.println("WiFi reconnected");
        Serial.println("New IP address: ");
        Serial.println(WiFi.localIP());
        break;
      }
      delay(50);
    }
    delay(1000);
    webserver_setup();
  }

  String response_message = "";
  response_message += "<html>";
  response_message += "<head><title>ESP8266 Webserver</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>WLAN Settings</center></h1>";

  response_message += "<ul><li><a href=\"/\">Return to main page</a></li></ul>";

  if (WiFi.status() == WL_CONNECTED)
  {
    response_message += "Status: Connected<br>";
  }
  else
  {
    response_message += "Status: Disconnected<br>";
  }

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
      response_message += "<input type=\"radio\" name=\"ssid\" value=\"" + String(WiFi.SSID(ap_idx)) + "\">";
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

bool bConnect = true;

void _DummyConnect(bool bConnect)
{
}

/*///////////////////////////////////////////////////////////////////////////*/
#define _USE_BUTTON_  1

void serverPageHandler()
{
  // Check if there are any GET parameters
  if (webserver.hasArg("server"))
  {
    serverIP.fromString(webserver.arg("server").c_str());
  }

  if (webserver.hasArg("gpio2"))
  {
#if _USE_BUTTON_
    if (webserver.arg("gpio2") == "Connect")
#else
    if (webserver.arg("gpio2") == "1")
#endif    
    {
      bConnect = true;
    }
    else
    {
      bConnect = false;
    }
    serverConnect(bConnect);
  }

  String response_message = "<html><head><title>ESP8266 Webserver</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>Configure Server</center></h1>";
  response_message += "<form method=\"get\">";

  response_message += "<ul><li><a href=\"/\">Return to main page</a></li></ul>";

  if (wificlient.connected())
  {
    response_message += "Status: Connected to server<br><br>";
    bConnect = true;
  }
  else
  {
    response_message += "Status: Disconnected from server<br><br>";
    bConnect = false;
  }
  
  response_message += "Server Address:<br>";
  response_message += "<input type=\"text\" value=\""+serverIP.toString()+"\"name=\"server\"><br><br>";
  
  response_message += "Server Connect:<br>";
  
  if (bConnect == false)
  {
#if _USE_BUTTON_
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"1\" onclick=\"submit();\">On<br>";
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"0\" onclick=\"submit();\" checked>Off<br>";
#else   
    response_message += "<input type=\"submit\" name=\"gpio2\" value=\"Connect\"><br>";
#endif    
  }
  else
  {
#if _USE_BUTTON_
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"1\" onclick=\"submit();\" checked>On<br>";
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"0\" onclick=\"submit();\">Off<br>";
#else   
    response_message += "<input type=\"submit\" name=\"gpio2\" value=\"Disconnect\"><br>";
#endif    
  }

  response_message += "</form></body></html>";

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

