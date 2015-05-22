#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "IPAddress.h"

#define WIFI_SSID "Your Themostat"
#define WIFI_PASS "coolthermostat"

#define MAX_WIFI_CONNECT_WAIT 60

#define WIFI_FORM "<html>"\
                  "<form method='post' action='/register'>"\
                  "<label>SSID</label>"\
                  "<input type='text' name='ssid'>"\
                  "<label>Password</label>"\
                  "<input type='password' name='pass'>"\
                  "<input type='submit' value='Connect'>"\
                  "</form>"\
                  "</html>"

#define STATUS_PAGE "<html>"\
                    "<p>Wifi status: [tmp]</p>"\
                    "</html>"

char wifi_status[15];

ESP8266WebServer server(80);

char* get_status_string(uint8_t status) {
  if(status == WL_CONNECTED)
      return "connected";
    else if (status == WL_NO_SSID_AVAIL)
      return "wrong SSID";
    else if (status == WL_CONNECT_FAILED)
      return "invalid password";
    else if (status == WL_IDLE_STATUS)
      return "idle";
    else
      return "disconnected";
}

// ================== HELPER FUNCTIONS ==================
uint8_t connect_to_wifi(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    uint8_t count = 0;
    while(WiFi.status() != WL_CONNECTED && count < MAX_WIFI_CONNECT_WAIT) {
      delay(500);
      count ++;
    }

    strcpy(wifi_status, get_status_string(WiFi.status()));
    return WiFi.status() == WL_CONNECTED;
}

// ================== REQUEST HANDLERS ==================
void handle_registration_req() {
  if(server.method() == HTTP_GET) {
    server.send(200, "text/html", WIFI_FORM);
  } else if(server.method() == HTTP_POST) {
    const char* ssid = server.arg("ssid").c_str();
    const char* pass = server.arg("pass").c_str();

    Serial.println(ssid);
    Serial.println(pass);
    connect_to_wifi(ssid, pass);
  }
}

void handle_temperature_req() {
  server.send(200, "text/plain", "temperature and humidity here");
}

void handle_status_req() {
  server.send(200, "text/plain", wifi_status);
}

// ================== SETUP FUNCTIONS ==================
void setup_wifi() {
  IPAddress local_gateway_ip(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAP(WIFI_SSID, WIFI_PASS, 2);
  WiFi.config(local_gateway_ip, local_gateway_ip, subnet);
}

void setup()
{
  Serial.begin(19200);
  WiFi.mode(WIFI_AP_STA);

  WiFi.softAP(WIFI_SSID, WIFI_PASS, 16);
  delay(1000);
  server.on("/register", handle_registration_req);
  server.on("/status", handle_status_req);
  server.on("/", handle_temperature_req);

  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  server.handleClient();
}

