#include <Arduino.h>
#include "FS.h"
#include "SPIFFS.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Communication.h"
#include <ESPmDNS.h>
#include <jled.h>

#define FORMAT_SPIFFS_IF_FAILED true
#define SERIAL_DEBUG Serial
#define SERIAL_DEBUG_BAUD 115200

#define SSID "rAhUl-eCvT"
#define PASSWORD "123456789"

// Serial port used for getting incoming data from teensy.
#define SERIAL_DATA Serial2
#define SERIAL_DATA_BAUD 115200

IPAddress apIP(192, 168, 1, 1);    // Private network for server
JLed led(LED_BUILTIN);
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
    SERIAL_DEBUG.begin(SERIAL_DEBUG_BAUD);
    SERIAL_DATA.begin(SERIAL_DATA_BAUD); // TEENSY BAUDRATE - CHANGE AS NEEDED.

    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
        SERIAL_DEBUG.println("SPIFFS Mount Failed");

        led.Blink(100, 100).Forever();
        while (true) led.Update();
    }

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    if (!MDNS.begin("ecvt")) {
        Serial.println("Error starting mDNS");
        led.Blink(100, 100).Blink(1000, 1000).Forever();
        while (true) led.Update();
    }

    WiFi.disconnect();
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

#ifdef PASSWORD
    WiFi.softAP(SSID);
#else
    WiFi.softAP(SSID, PASSWORD);
#endif

    Serial.println(WiFi.softAPIP());

    server.begin();
    Serial.println("BEGAN");
    led.Blink(500, 500).Forever();
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

void loop() {
    static uint8_t comm_buf[1024];
    static size_t comm_size;
    static CommState state = INITIALIZE;

    switch (state) {
        case INITIALIZE:
            state = WAIT_PACKET_FILL;
            break;

        case WAIT_PACKET_FILL:
            if (SERIAL_DATA.available())
                comm_buf[comm_size++] = SERIAL_DATA.read();

            if (comm_size > TOTAL_SIZE)
                state = TRY_VALIDATE_PACKET;

            break;

        case TRY_VALIDATE_PACKET:
            Serial.println("VALIDATING");
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
            comm_size -= TOTAL_SIZE;
            state = WAIT_PACKET_FILL;
            break;
    }

    led.Update();
}