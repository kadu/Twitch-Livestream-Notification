#include <Adafruit_NeoPixel.h>
#ifdef ESP32
#include <ESPmDNS.h>
#include <WebServer.h>
#else
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#endif
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager


#include <EEPROM.h>
#include <ArduinoJson.h>
#define EEPROM_SIZE 512
int metaAddress = 0;
int metaLenght = 4;
int jsonAddress = 4;
String EEPROM_read(int index, int length) {
  String text = "";
  char ch = 1;

  for (int i = index; (i < (index + length)) && ch; ++i) {
    if (ch = EEPROM.read(i)) {
      text.concat(ch);
    }
  }
  return text;
}

int EEPROM_write(int index, String text) {
  for (int i = index; i < text.length() + index; ++i) {
    EEPROM.write(i, text[i - index]);
  }
  EEPROM.write(index + text.length(), 0);
  EEPROM.commit();
  Serial.println("alterações foram salvas na memoria!");

  return text.length() + 1;
}

DynamicJsonDocument getEEPROM_JSON() {

  String jsonRead = EEPROM_read(jsonAddress,
                                EEPROM_read(metaAddress, metaLenght).toInt());

  Serial.print("JSON Read: ");
  Serial.println(jsonRead);

  DynamicJsonDocument jsonDoc(EEPROM_SIZE);

  deserializeJson(jsonDoc, jsonRead);

  return jsonDoc;
}

void setEEPROM_JSON(DynamicJsonDocument jsonDoc) {

  String jsonWriteString;

  serializeJson(jsonDoc, jsonWriteString);

  // Serial.print("JSON Write: ");
  // Serial.println(jsonWriteString);

  EEPROM_write(metaAddress,
               (String)EEPROM_write(jsonAddress, jsonWriteString));
}


	
#include <cstring>
#include <sstream>
#include <string>

#include "./HTML.h"
#include "./style.h"
#include "./js.h"
#include "./twitch.hpp"

// LED config
#ifdef ESP32
#define LED_R D12
#define LED_G D2
#define LED_B D4
#else
#define LED_R 14
#define LED_B 13
#define LED_G 12
#define GND D0
#endif
#define PIN 4
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
int status = 0;

// WifiManager config
void wmConfig() {
    // WiFiManager, Local intialization. Once its business is done, there is no
    // need to keep it around
    WiFiManager wm;
    // wm.resetSettings(); // reset the wifi config

    bool res;
    res = wm.autoConnect("StarON");  // password protected ap

    if (!res) {
        // Serial.println("Failed to connect");
        //  ESP.restart();
        Serial.println("not connected...");
  delay(150);
  digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));

    } else {
        // if you get here you have connected to the WiFi
         Serial.println("connected...yeey :)");
    }
     Serial.println(WiFi.localIP());
}

// WebServer config
#ifdef ESP32
WebServer server(80);  // Set web server port number to 80
#else
ESP8266WebServer server(80);  // Set web server port number to 80
#endif
String streamerName = "";
String cor = "";
int corR = 0;
int corG = 0;
int corB = 0;
int modo = 0;

void hendleIndex() {                           // send HTML to the page
    Serial.println("GET /");
    server.send(200, "text/html", postForms);  // check HTML.h file
}

void handleStyle() {                           // send HTML to the page
    Serial.println("GET /style.css");
    server.send(200, "text/css", style);  // check HTML.h file
}

void handlejs() {                           // send HTML to the page
    Serial.println("GET /js");
    server.send(200, "application/javascript", js);  // check HTML.h file
}


void handleStatus() {                           // send JSON to the page
//jsonstatus = "[{\"canal\":\""+streamerName+"\",\"color\":\""+cor+"\",\"status\":\""+status+"\"}]";   
    Serial.println("GET /staus");
    DynamicJsonDocument jsonDoc = getEEPROM_JSON();    
    const char *readcanal = jsonDoc["canal"];
    const char *readcor = jsonDoc["cor"];
   //int r = jsonDoc["rgb"][0];
   //int g = jsonDoc["rgb"][1];
   //int b = jsonDoc["rgb"][2];





    server.send(200, "application/json", "[{\"canal\":\""+String(readcanal)+"\",\"color\":\""+String(readcor)+"\",\"status\":\""+status+"\"}]"); 
}


void handleGetParam() {

    if (server.hasArg("STREAMER")) {
        streamerName = server.arg("STREAMER");  // get the streamer name and put
                                                // on the streamerName variable
    }
    if (server.hasArg("cor")) {
        cor = server.arg("cor");  // get the COLOR
    }


     if (server.hasArg("r")) {
        corR = server.arg("r").toInt();  // get the COLOR
        Serial.println(corR);
    }
     if (server.hasArg("g")) {
        corG = server.arg("g").toInt();  // get the COLOR
        Serial.println(corG);
    }

 if (server.hasArg("b")) {
        corB = server.arg("b").toInt();  // get the COLOR
        Serial.println(corB);
    }

if (server.hasArg("STREAMER")) {
DynamicJsonDocument jsonDoc(EEPROM_SIZE);
  jsonDoc["canal"] = server.arg("canal");
  jsonDoc["cor"] = server.arg("cor");
  jsonDoc["rgb"][0] = server.arg("r").toInt();
  jsonDoc["rgb"][1] = server.arg("g").toInt();
  jsonDoc["rgb"][2] = server.arg("b").toInt();

  setEEPROM_JSON(jsonDoc);
}




    for (int i = 0; i < 3; i++) { // piscar 3 vezes mostrando a cor escolhida
        
        server.handleClient();
        analogWrite(LED_R, corR);
        analogWrite(LED_G, corG);
        analogWrite(LED_B, corB);
        pixels.setPixelColor(0, corR, corG, corB);
        pixels.show();        
        delay(200);

        server.handleClient();
        analogWrite(LED_R, 0);
        analogWrite(LED_G, 0);
        analogWrite(LED_B, 0);
        pixels.clear();
        pixels.show();
        delay(200);
    }

  Serial.println("GET /getname");
    Serial.print("Streamer: ");
    Serial.print(streamerName);
    Serial.print(" - ");
    Serial.print("color: " + cor + " rgb("+server.arg("r")+", "+server.arg("g")+", "+server.arg("b")+")");
    Serial.println("");

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
    server.send(200, "text/html", message);  // check HTML.h file
}

void setup() {
  #ifdef ESP8266
  // se for esp8266, definir a frequencia do pwm em "mais de 8mil" hertz (isso ajuda a não flikar na camera)
  // essa opção é mais pra quem for usar o staron com led rgb não endereçável, ou ligar ele numa fita não endereçável
  analogWriteFreq(8001);   
  #endif


  EEPROM.begin(EEPROM_SIZE);
  DynamicJsonDocument jsonDoc(EEPROM_SIZE);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(GND, OUTPUT);
  digitalWrite(GND,0);
  digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
  delay(150);
  digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
  delay(150);
  digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
  delay(150);
  digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
  delay(150);
  digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
  delay(150);
  digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
  delay(150);
  digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
   
    WiFi.mode(WIFI_STA);
    Serial.begin(115200);
    wmConfig();
    if (!MDNS.begin("starOn")) {  // Start the mDNS responder for esp8266.local
        Serial.println("Error setting up MDNS responder!");
    }
    // Serial.println("mDNS responder started");
    MDNS.addService("http", "tcp", 80);
    WiFi.hostname("starOn");

    server.on("/", hendleIndex);
    server.on("/getname", handleGetParam);
    server.on("/style.css", handleStyle);
    server.on("/status", handleStatus);
    server.on("/js", handlejs);
    server.onNotFound(handleNotFound);
    server.begin();
    // Serial.println("HTTP server started");

    // LED

    pixels.begin();
    pixels.clear();
    
   jsonDoc = getEEPROM_JSON();    
   streamerName = String(jsonDoc["canal"]);
   cor =  String(jsonDoc["cor"]);
   corR = String(jsonDoc["rgb"][0]).toInt();
   corG = String(jsonDoc["rgb"][1]).toInt();
   corB = String(jsonDoc["rgb"][2]).toInt();
   modo = String(jsonDoc["modo"]).toInt();

}

uint32_t lasTimeUpdateLed;
uint32_t piscante = 0;
void loop() {
 // Serial.print("testando");
#ifndef ESP32
    MDNS.update();
#endif

    server.handleClient();

    if (streamerName != "" && (millis() - lasTimeUpdateLed) > 300) {
        // Serial.println("Recebendo stream data");
        // Serial.println(response);
        if (streamerIsOn(streamerName)) {
            server.handleClient();

            analogWrite(LED_R, corR);
            analogWrite(LED_G, corG);
            analogWrite(LED_B, corB);
            digitalWrite(GND, 0);
            
            pixels.setPixelColor(0, corR, corG, corB); //definir cor dos leds
            pixels.show(); //aplicar alterações nos leds
            Serial.println("TA ON");
            status = 1;
            digitalWrite(LED_BUILTIN, 0);
        } else {
            analogWrite(LED_R, 0);
            analogWrite(LED_G, 0);
            analogWrite(LED_B, 0);
            digitalWrite(GND, 0);
            server.handleClient();
            pixels.clear();//definir que quer apagar leds
            pixels.show(); //aplicar alterações nos leds
            Serial.println("TA OFF");
            status = 0;
            digitalWrite(LED_BUILTIN, 1);
        }
        lasTimeUpdateLed = millis();
        // Serial.println("wait 5 sec...");
        // delay(5000);
    }
 if (streamerName == ""){
   pulsar();
 }

}


const int LED_PIN = LED_BUILTIN; //LED DO ESP para feedback
const int PERIODO = 1000; // período em milissegundos
int contador = 0; // contador para o software timer
int intensidade = 0; // intensidade do sinal PWM
int direcao = 1; // direção do aumento da intensidade


void pulsar(){
   // altera a intensidade do sinal PWM de acordo com o software timer
  if (contador == 0) {
    // se o contador estiver zerado, inicia o aumento da intensidade
    intensidade = 0;
    direcao = 1;
  } else if (contador == PERIODO/2) {
    // se o contador atingir metade do período, inicia a diminuição da intensidade
    direcao = -1;
  } else if (contador == PERIODO || intensidade == 0) {
    // se o contador atingir o período ou a intensidade chegar a zero, reseta o contador e a direção
    contador = 0;
    direcao = 1;
  }
  // aplica a intensidade atual ao sinal PWM
  analogWrite(LED_R, corR);
  analogWrite(LED_G, corG);
  analogWrite(LED_B, corB);
  analogWrite(GND, intensidade);
  analogWrite(LED_PIN, intensidade);
  intensidade += direcao; // incrementa ou decrementa a intensidade
  contador++; // incrementa o contador
  delay(4); // espera por 4 milissegundo antes de continuar o loop
}
