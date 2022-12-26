#if !defined(TWITCH_HPP__)
#define TWITCH_HPP__
#include <Arduino_JSON.h>
#ifndef ESP32
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#else
#include <WiFi.h>
#endif

#include "./credencials.h"

// WiFiClientSecure client;
// twitch connection
const char* host = "id.twitch.tv";
const uint16_t port = 443;
#define LENACESSTOKEN 64
char access_token[LENACESSTOKEN] = {0};

uint32_t lasTimeGetToken = 0;
uint32_t latTimeGetStreamerOn = 0;
bool isStreamerOn = false;

StaticJsonDocument<1024> doc;

bool awaitTimeOut(WiFiClientSecure* client) {
    // read back one line from server
    // Serial.println("receiving from remote server");
    delay(1000);
    unsigned long timeout = millis();
    while (client->available() < 64) {
        if ((millis() - timeout) > 5000) {
            Serial.println(">>> Client Timeout !");
            client->stop();
            return 1;
        }
    }
    return 0;
}

bool handGetTwitchToken(char* ass_) {
    BearSSL::WiFiClientSecure myClient;
    HTTPClient https;

    myClient.setInsecure();

    if (!https.begin(myClient, "https://id.twitch.tv/oauth2/token")) {
        Serial.println("Unable to connect");
        return false;
    }

    https.addHeader("Content-Type", "application/x-www-form-urlencoded");

    char data[256];

    sprintf(data, "client_id=%s&client_secret=%s&grant_type=client_credentials",
            clientId, clientSecret);

    // String data = "client_id=" + String(clientId) +
    //               "&client_secret=" + String(clientSecret) +
    //               "&grant_type=client_credentials";

    int httpCode = https.POST(data);

    if (httpCode == 0) {
        Serial.printf("[HTTPS] POST... failed, error: %s\r",
                      https.errorToString(httpCode).c_str());
        Serial.println("Error on HTTP request");
        return false;
    }
    Serial.printf("[HTTPS] POST... code: %d len: %d \r", httpCode,
                  https.getSize());

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();

        auto error = deserializeJson(doc, payload);
        if (error) {
            Serial.println("deserializeJson() failed: " +
                           String(error.c_str()));
            return false;
        }

        const char* acc = doc["access_token"];
        strcpy(ass_, acc);
        // Serial.println(access_token);
        return true;
    }

    return false;
}

#define IS_ACESS_TOKEN_VALID() (strlen(access_token) > 0)

bool getTwitchToken() {
    if (!WiFi.isConnected()) return 0;
    if (lasTimeGetToken && (millis() - lasTimeGetToken) <
                               (IS_ACESS_TOKEN_VALID() ? 60000 : 3000)) {
        return IS_ACESS_TOKEN_VALID();
    }

    if (!handGetTwitchToken(access_token)) {
        memset(access_token, 0, LENACESSTOKEN);
    }

    lasTimeGetToken = millis();
    return IS_ACESS_TOKEN_VALID();
}

bool handStreamerIsOn(const char* streamerName) {
        BearSSL::WiFiClientSecure myClient;
    HTTPClient https;

    myClient.setInsecure();
    char url[256];

    sprintf(url, "https://api.twitch.tv/helix/streams?user_login=%s",
            streamerName);

    if (!https.begin(myClient, url)) {
        Serial.println("2 connection failed");
        Serial.println("wait 5 sec...");
        // delay(5000);
        return false;
    }

    https.addHeader("Authorization", "Bearer " + String(access_token));
    https.addHeader("Client-Id", clientId);
    // This will send the request to the server
    // client.println("GET /helix/streams?user_login=" + streamerName +
    //                " HTTP/1.1");
    // client.println("Host: api.twitch.tv");
    // client.println("Authorization: Bearer ");
    // client.println(access_token);
    // client.println("Client-Id: ");
    // client.println(clientId);
    // client.println();
    // client.println();

    int httpCode = https.GET();

    if (httpCode == 0) {
        Serial.printf("[HTTPS] GET... failed, error: %s\r",
                      https.errorToString(httpCode).c_str());
        Serial.println("Error on HTTP request");
        return false;
    }

    Serial.printf("[HTTPS] GET... code: %d len: %d \r", httpCode,
                  https.getSize());

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();

        auto error = deserializeJson(doc, payload);
        if (error) {
            Serial.println("deserializeJson() failed: " +
                           String(error.c_str()));
            return false;
        }

        const char* user_name = doc["data"][0]["user_name"];
        Serial.println(user_name);
        return user_name != NULL;
    }

    return false;
}

bool streamerIsOn(String streamerName) {
    if (getTwitchToken()) {
        if ((millis() - latTimeGetStreamerOn) < 3000) return isStreamerOn;
        isStreamerOn = handStreamerIsOn(streamerName.c_str());
        latTimeGetStreamerOn = millis();
        return isStreamerOn;
    }
    return false;
}

#endif  // TWITCH_HPP__
