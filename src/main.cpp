#include <Arduino.h>
#include <DeskControllerFactory.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>

#include "credentials.h" // for bt_mac_address

AsyncWebServer server(80);

auto controller = LinakDesk::DeskControllerFactory::make();

void moveToHeightHttpHandler(AsyncWebServerRequest* request) {
    if (request->hasParam("destination")) {
        auto message = request->getParam("destination")->value();
        auto destination = message.toInt();
        if (controller.isConnected() && destination >= 0 && destination < 7000) {
            request->send(200, "text/plain", "Moving to: " + message);
            controller.moveToHeight(destination);
            return;
        }
    }
    request->send(400, "text/plain", "Wrong input");
}

void notFound(AsyncWebServerRequest* request) { request->send(404, "text/plain", "Not found"); }

void setup() {
    Serial.begin(115200);
    Serial.print("Connecting to desk: ");
    Serial.println(bt_mac_address);
    if (controller.connect(bt_mac_address)) {
        // Sanity check to see if the desk connection works
        auto before = millis();
        Serial.printf("Current height: %d\n", controller.getHeight());
        Serial.printf("Getting height and printing it took: %ldms\n", millis() - before);

        WiFi.begin(ssid, password);
        Serial.println("Connecting to WiFi");
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.print(".");
        }
        Serial.println("\nWiFi connected!");
        Serial.print("Got IP: ");
        Serial.println(WiFi.localIP());
        // respond to GET requests on URL /heap
        server.on("/heap", HTTP_GET,
                  [](AsyncWebServerRequest* request) { request->send(200, "text/plain", String(ESP.getFreeHeap())); });
        server.on("/moveToHeight", HTTP_GET, moveToHeightHttpHandler);
        server.on("/getHeight", HTTP_GET, [](AsyncWebServerRequest* request) {
            request->send(200, "text/plain", String(controller.getHeight()).c_str());
        });
        server.onNotFound(notFound);

        MDNS.begin("linak-desk");
        MDNS.addService("http", "tcp", 80);
        server.begin();
        Serial.println("HTTP server started");
    } else {
        Serial.println("Couldn't connect to desk!");
    }
}

void loop() {
    delay(10);
    controller.loop();
}