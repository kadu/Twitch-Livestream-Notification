#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WiFiClientSecure.h>
#include <Arduino_JSON.h>
#include <Adafruit_NeoPixel.h>
#include <sstream>
#include <string>
#include <cstring>

// Set web server port number to 80
ESP8266WebServer server(80);

// twitch connection
const String clientId = "";
const String clientSecret = "";
const char* host = "id.twitch.tv";
const uint16_t port = 443;
String access_token = "";

//LED 
#define PIN 2
#define NUMPIXELS 2 
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

String stremaerName="";
const String postForms = R"===(
<html>
    <head>
        <title>Set Streamer</title>
        <style>
        body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }
        </style>
    </head>
    <body>
        <h1>POST plain text to /getName/</h1><br>
        <form method="GET" action="/getName">
        <input type="text" name="STREAMER">
        <input type="submit" value="Submit\">
        </form>
    </body>
</html>
)===";

void hendleIndex() {  // funcao que vai enviar o HTML
    server.send(200, "text/html", postForms);
}

void handleGetParam() {

    if (server.hasArg("STREAMER")) {
        stremaerName = server.arg("STREAMER"); // AGORA TA CERTO
    }
    Serial.print("Stremaer Name - ");
    Serial.println(stremaerName);
}

void handleNotFound() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}

void wmConfig(){

    
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;
    wm.resetSettings(); // reset the wifi config
    
    bool res;
    res = wm.autoConnect("StarON"); // password protected ap
 
    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }
        Serial.println(WiFi.localIP());
}

void setup() {
    WiFi.mode(WIFI_STA); 
    Serial.begin(115200);
    wmConfig();

    server.on("/", hendleIndex);
    server.on("/getName", handleGetParam);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");

    //LED
    pixels.begin();
    pixels.clear();
    delay(500);

}
 
void loop() {
  server.handleClient();
  WiFiClientSecure client;
  client.setInsecure();
  if(stremaerName != ""){

  if (!client.connect(host, port)) {
    Serial.println("1 connection failed");
    Serial.println("wait 5 sec...");
    delay(5000);
    return;
  }  

  String data = "client_id=" + clientId + "&client_secret=" + clientSecret + "&grant_type=client_credentials";
  
//   This will send the request to the server
  client.println("POST /oauth2/token HTTP/1.1");
  client.println("Host: id.twitch.tv");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.println("Content-Length: " + String(data.length()));
  client.println();
  client.println(data);


  //read back one line from server
  Serial.println("receiving from remote server");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      delay(60000);
      return;
    }
  }

  std::stringstream ss;
  bool capturing = false;
  while (client.available()) {
    char ch = static_cast<char>(client.read());
    if(ch == '{' || capturing == true || ch == '}') {
      capturing = true;
      ss << ch;
      if(ch == '}') {
        capturing = false;
      }
    }    
  }  

  JSONVar json = JSON.parse(ss.str().c_str());  

  access_token = String((const char*) json["access_token"]);
  Serial.println(access_token);
  
  if (!client.connect("api.twitch.tv", port)) {
    Serial.println("2 connection failed");
    Serial.println("wait 5 sec...");
    delay(5000);
    return;
  }  

  Serial.println("GET /helix/streams?user_login="+stremaerName+ " HTTP/1.1");
  // This will send the request to the server
  client.println("GET /helix/streams?user_login="+stremaerName+ " HTTP/1.1");
  client.println("Host: api.twitch.tv"); 
  client.println("Authorization: Bearer " + access_token);
  client.println("Client-Id: " + clientId);
  client.println();
  client.println();

  timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 10000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      delay(60000);
      return;
    }
  }
  ss.clear();

  delay(1000);

  capturing = false;
  String response = "";
  while (client.available()) {    
    char ch = static_cast<char>(client.read());
    if(ch == '{' || capturing == true || ch == '}') {
      capturing = true;
      response.concat(ch);
      if(ch == '}') {
        capturing = false;
      }
    }
  }

  Serial.println("Recebendo stream data");
  Serial.println(response);
  if (response.length() > 27){
    pixels.setPixelColor(1, pixels.Color(254, 0, 153));
    pixels.show();   
    Serial.println("TA ON");
  }else{
     pixels.setPixelColor(1, pixels.Color(0, 0, 255));
     pixels.show(); 
    Serial.println("TA OFF");
    
  }



  Serial.println("closing connection");
  client.stop();

  Serial.println("wait 5 sec...");
  delay(5000);
  }
  }