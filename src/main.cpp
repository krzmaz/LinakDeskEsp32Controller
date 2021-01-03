#include <Arduino.h>

#include <BluetoothConnection.h>

#include "credentials.h" // for bt_mac_address

IRAM_ATTR static void printStringAsHex(const std::string& input, bool breakLine = true) {
    for (const auto& ch : input) {
        if (ch < 0x10) {
            Serial.print('0');
        }
        Serial.print(ch, HEX);
        Serial.print(' ');
    }
    if (breakLine) {
        Serial.println();
    }
}

IRAM_ATTR static void printingNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData,
                                             size_t length, bool isNotify) {
    Serial.print("Notify callback for characteristic: ");
    Serial.println(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print("Data: ");
    printStringAsHex(std::string((char*)pData, length));
}

// class MyClientCallbacks : public BLEClientCallbacks {
//   public:
//     void onConnect(BLEClient* pClient) { Serial.println("onConnect()"); }
//     void onDisconnect(BLEClient* pClient) { Serial.println("onDisconnect()"); }
// };

void setup() {
    Serial.begin(115200);

    // client->setClientCallbacks(new MyClientCallbacks());
    Serial.println("connecting");
    std::unique_ptr<LinakDesk::ConnectionInterface> deskConnection = std::make_unique<LinakDesk::BluetoothConnection>(std::string(bt_mac_address));

    if (deskConnection->isConnected()) {
        Serial.println("connected");
        Serial.println("Trying the move: ");

        deskConnection->startMoveTorwards();

        uint16_t destination = 700; // raw destination height - 700 is quite low, look out!

        // TODO (#1): this needs a mechanism for breaking out of the loop if the desk hits something
        while (deskConnection->getHeight() != destination) {
            deskConnection->moveTorwards(destination);
            const TickType_t delay = 200 / portTICK_PERIOD_MS;
            vTaskDelay(delay);
        }
    } else {
        Serial.println("not connected!");
    }
}

void loop() {
    delay(1000);

    // Serial.println(client->isConnected());
}