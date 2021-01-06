#include <Arduino.h>

#include <DeskControllerFactory.h>

#include "credentials.h" // for bt_mac_address


void setup() {
    Serial.begin(115200);

    // client->setClientCallbacks(new MyClientCallbacks());
    Serial.println("connecting");
    auto controller = LinakDesk::DeskControllerFactory::make();
    controller.connect(bt_mac_address);

    if (controller.isConnected()) {
        Serial.println("connected");
        // deskConnection->attachHeightSpeedCallback(printingCallback);
        Serial.println("Trying the move: ");

        uint16_t destination = 700; // raw destination height - 700 is quite low, look out!

        controller.moveToHeight(destination);

    } else {
        Serial.println("not connected!");
    }
}

void loop() {
    delay(1000);

    // Serial.println(client->isConnected());
}