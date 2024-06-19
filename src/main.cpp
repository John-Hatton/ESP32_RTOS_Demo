

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

const char* ssid = "SSID";
const char* password = "PASSWORD";

AsyncWebServer server(80);

TaskHandle_t blinkTaskHandle = NULL;
const int ledPin = 2; // Use GPIO 2 for the LED

void connectToWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void blinkTask(void *pvParameter) {
    int pin = *((int *)pvParameter);
    pinMode(pin, OUTPUT);
    Serial.println("Blink task started");
    while (1) {
        digitalWrite(pin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));
        digitalWrite(pin, LOW);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void startBlinking() {
    if (blinkTaskHandle == NULL) {
        Serial.println("Starting blink task");
        xTaskCreate(blinkTask, "blinkTask", 4096, (void*)&ledPin, 1, &blinkTaskHandle);
    }
}

void stopBlinking() {
    if (blinkTaskHandle != NULL) {
        Serial.println("Stopping blink task");
        vTaskDelete(blinkTaskHandle);
        blinkTaskHandle = NULL;
        digitalWrite(ledPin, LOW); // Turn off the LED
    }
}

void listSPIFFSFiles() {
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
        Serial.print("FILE: ");
        Serial.println(file.name());
        file = root.openNextFile();
    }
}

void logRequest(AsyncWebServerRequest *request, int code) {
    Serial.printf("Client IP: %s, Requested URL: %s, Response Code: %d\n",
                  request->client()->remoteIP().toString().c_str(),
                  request->url().c_str(),
                  code);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Setup started");
    connectToWiFi();

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
    Serial.println("SPIFFS mounted successfully");
    listSPIFFSFiles();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("Request for / received");
        if(SPIFFS.exists("/index.html")) {
            request->send(SPIFFS, "/index.html", "text/html");
            logRequest(request, 200);
        } else {
            Serial.println("index.html not found");
            request->send(404, "text/plain", "index.html not found");
            logRequest(request, 404);
        }
    });

    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("Request for /styles.css received");
        if(SPIFFS.exists("/styles.css")) {
            request->send(SPIFFS, "/styles.css", "text/css");
            logRequest(request, 200);
        } else {
            Serial.println("styles.css not found");
            request->send(404, "text/plain", "styles.css not found");
            logRequest(request, 404);
        }
    });

    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("Request for /script.js received");
        if(SPIFFS.exists("/script.js")) {
            request->send(SPIFFS, "/script.js", "application/javascript");
            logRequest(request, 200);
        } else {
            Serial.println("script.js not found");
            request->send(404, "text/plain", "script.js not found");
            logRequest(request, 404);
        }
    });

    server.on("/StartBlinking", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("Request for /StartBlinking received");
        startBlinking();
        request->send(200, "text/plain", "Started Blinking");
        logRequest(request, 200);
    });

    server.on("/StopBlinking", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("Request for /StopBlinking received");
        stopBlinking();
        request->send(200, "text/plain", "Stopped Blinking");
        logRequest(request, 200);
    });

    server.onNotFound([](AsyncWebServerRequest *request){
        Serial.print("Not Found: ");
        Serial.println(request->url());
        request->send(404, "text/plain", "Not found");
        logRequest(request, 404);
    });

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    // Do nothing in the loop, everything is handled by tasks and server
}