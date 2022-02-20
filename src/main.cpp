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

#define SSID "rahul-dyno"
//#define PASSWORD "123456789"

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

        led.Blink(100, 1000).Forever();
        while (true) led.Update();
    }

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    WiFi.disconnect();
    WiFi.config(
            IPAddress(192, 168, 1, 2),
            IPAddress(192, 168, 1, 1),
            IPAddress(255, 255, 255, 0)
    );
    WiFi.setHostname("dyno");

#ifndef PASSWORD
    WiFi.begin(SSID);
#else
    WiFi.begin(SSID, PASSWORD);
#endif
    led.Blink(100, 500).Forever();
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        led.Update();
    }
    Serial.println(WiFi.localIP());

    server.begin();
    Serial.println("BEGAN");
    led.Blink(1000, 1000).Forever();
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
unsigned long lastNotif = 0;

const int DISPATCH_SIZE = 1500;
const long int DISPATCH_TIMEOUT = 1000;

void loop() {
    static uint8_t comm_buf[DISPATCH_SIZE + 5];
    static size_t comm_size = 0;
    static CommState state = INITIALIZE;

    static char msg_buf[1024];
    static unsigned long serial_bytes = 0;
    static unsigned long last_write = 0;

    switch (state) {
        case INITIALIZE:
            state = WAIT_PACKET_FILL;
            break;

        case WAIT_PACKET_FILL:
            if (SERIAL_DATA.available()) {
                comm_buf[comm_size++] = SERIAL_DATA.read();
                serial_bytes++;
            }

            if (comm_size >= DISPATCH_SIZE || (millis() - last_write > DISPATCH_TIMEOUT && comm_size > TOTAL_SIZE))
                state = REPLICATE_FRAME;

            break;

        case REPLICATE_FRAME:
            ws.binaryAll(comm_buf, comm_size);
            comm_size = 0;
            last_write = millis();
            state = WAIT_PACKET_FILL;
            break;
    }

    if (millis() - lastNotif > 1000) {
        sprintf(msg_buf, "RX: %ld", serial_bytes);
        ws.textAll(msg_buf);
        lastNotif = millis();
    }

    led.Update();
}