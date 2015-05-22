#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

/* Set these to your desired credentials. */
const char* ssid = "Your Thermostat";
const char* password = "coolthermostat";

const char* wifi_network_ssid = "Clicknet-FF21";
const char* wifi_network_password = "WXYMT4XD63CH4";

// multicast DNS responder
MDNSResponder mdns;

// TCP server at port 80 will respond to HTTP requests
WiFiServer web_server(80);
ESP8266WebServer wifi_server(8080);


void setup_mdns()
{
  if (!mdns.begin("thermostat", WiFi.localIP())) {
    Serial.println("Error setting up MDNS responder!");
    while(1) { 
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
 
  // Start TCP (HTTP) server
  web_server.begin();
  Serial.println("TCP server started");
}

void serve_network_client()
{
  WiFiClient client = web_server.available();
  if (!client) {
    return;
  }
  Serial.println("");
  Serial.println("New client");

  // Wait for data from client to become available
  while(client.connected() && !client.available()){
    delay(1);
  }
  
  // Read the first line of HTTP request
  String req = client.readStringUntil('\r');
  
  // First line of HTTP request looks like "GET /path HTTP/1.1"
  // Retrieve the "/path" part by finding the spaces
  int addr_start = req.indexOf(' ');
  int addr_end = req.indexOf(' ', addr_start + 1);
  if (addr_start == -1 || addr_end == -1) {
    Serial.print("Invalid request: ");
    Serial.println(req);
    return;
  }
  req = req.substring(addr_start + 1, addr_end);
  Serial.print("Request: ");
  Serial.println(req);
  client.flush();
  
  String s;
  if (req == "/")
  {
    IPAddress ip = WiFi.localIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
    s += ipStr;
    s += "</html>\r\n\r\n";
    Serial.println("Sending 200");
  }
  else
  {
    s = "HTTP/1.1 404 Not Found\r\n\r\n";
    Serial.println("Sending 404");
  }
  Serial.println(ESP.getFreeHeap());
  client.print(s);
  
  Serial.println("Done with client");
}

void connect_to_wifi()
{
  // Connect to WiFi network
  WiFi.begin(wifi_network_ssid, wifi_network_password);
  Serial.println("");  
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.print(WiFi.status());
  }
  Serial.println("");/
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_softap()
{
  Serial.print("Configuring access point...");\
  WiFi.softAP(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("done");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  wifi_server.on("/", handleRoot);
  wifi_server.begin();

  Serial.println("HTTP server started");
}

void handleRoot() {
	wifi_server.send(200, "text/html",
    "<html>"
    "<h4>Connected to wifi: %s</h4>");
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();

  setup_softap();
}

void loop() {
	wifi_server.handleClient();
}
