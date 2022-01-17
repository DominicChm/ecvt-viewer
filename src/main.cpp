#include <Arduino.h>
#include "FS.h"
#include "SPIFFS.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Communication.h"

#define FORMAT_SPIFFS_IF_FAILED true
#define SERIAL_OUT Serial

#include <ESPmDNS.h>
// DNS!
// https://gist.github.com/Cyclenerd/7c9cba13360ec1ec9d2ea36e50c7ff77

const byte DNS_PORT = 53;          // Capture DNS requests on port 53
IPAddress apIP(192, 168, 1, 1);    // Private network for server

const char *ssid = "ecvt_data";
const char *password = "123456789";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data,
               size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        client->printf("Hello Client %u :)", client->id());
        client->ping();
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
    } else if (type == WS_EVT_ERROR) {
        Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *) arg), (char *) data);
    } else if (type == WS_EVT_PONG) {
        Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *) data : "");
    } else if (type == WS_EVT_DATA) {
        Serial.println("DATA");
    }
}

void setup() {
    Serial.begin(115200);

    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(ssid);
    Serial.println(WiFi.softAPIP());

    if (!MDNS.begin("ecvt")) {
        Serial.println("Error starting mDNS");
        return;
    }

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    server.begin();
}


enum CommState {
    INITIALIZE,
    WAIT_PACKET_FILL,
    TRY_VALIDATE_PACKET,
    REPLICATE_FRAME
};
const uint8_t START_DATA_SIZE = 2;   // Bytes
const uint8_t START_BYTE_VAL = 0xAA; // 1010 1010
const int8_t CHECK_DATA_SIZE = 2;   // Bytes
const uint8_t TOTAL_SIZE = sizeof(Data) + START_DATA_SIZE + CHECK_DATA_SIZE;

unsigned long last_emitted = millis();

void loop() {
    static uint8_t comm_buf[1024];
    static size_t comm_size;
    static CommState state = INITIALIZE;

    switch (state) {
        case INITIALIZE:
            state = WAIT_PACKET_FILL;
            break;

        case WAIT_PACKET_FILL:
            if (SERIAL_OUT.available())
                comm_buf[comm_size++] = Serial.read();

            if (comm_size > TOTAL_SIZE)
                state = TRY_VALIDATE_PACKET;
            break;

        case TRY_VALIDATE_PACKET:
            if (comm_buf[0] == START_BYTE_VAL && comm_buf[1] == START_BYTE_VAL)
                state = REPLICATE_FRAME;
            else {
                memcpy(&comm_buf, &comm_buf[1], comm_size - 1);
                comm_size--;
                state = WAIT_PACKET_FILL;
            }
            break;

        case REPLICATE_FRAME:
            ws.binaryAll(comm_buf, TOTAL_SIZE);
            memcpy(&comm_buf, &comm_buf[TOTAL_SIZE], comm_size - TOTAL_SIZE);
            state = WAIT_PACKET_FILL;
            break;
    }

    if (millis() - 100 > last_emitted) {
        comm_buf[comm_size++] = 0xAA;
        last_emitted = millis();
    }
// write your code here
}