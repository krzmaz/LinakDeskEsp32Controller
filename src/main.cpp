#include <Arduino.h>
#include <BLEDevice.h>

#include "credentials.h" // for bt_mac_address

static BLEUUID nameServiceUUID("00001800-0000-1000-8000-00805f9b34fb");
static BLEUUID nameCharacteristicUUID("00002a00-0000-1000-8000-00805f9b34fb");
static BLEUUID controlServiceUUID("99fa0001-338a-1024-8a49-009c0215f78a");
static BLEUUID controlCharacteristicUUID("99fa0002-338a-1024-8a49-009c0215f78a");
static BLEUUID dpgServiceUUID("99fa0010-338a-1024-8a49-009c0215f78a");
static BLEUUID dpgCharacteristicUUID("99fa0011-338a-1024-8a49-009c0215f78a");
static BLEUUID outputServiceUUID("99fa0020-338a-1024-8a49-009c0215f78a");
static BLEUUID outputCharacteristicUUID("99fa0021-338a-1024-8a49-009c0215f78a");
static BLEUUID inputServiceUUID("99fa0030-338a-1024-8a49-009c0215f78a");
static BLEUUID inputCharacteristicUUID("99fa0031-338a-1024-8a49-009c0215f78a");

BLEClient* client;

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

// We need to query the name, otherwise the controller won't react
void queryName() {
    const TickType_t delay = 500 / portTICK_PERIOD_MS;
    vTaskDelay(delay); // aparently needed (by the linak controller maybe?)
    auto name = client->getService(nameServiceUUID)->getCharacteristic(nameCharacteristicUUID)->readValue();
    vTaskDelay(delay); // aparently needed (by the linak controller maybe?)
    Serial.println("Name:");
    Serial.println(name.c_str());
}

// mimic the calls done by LinakDeskApp after connection
void deskInit() {
    queryName();
    auto dpgChar = client->getService(dpgServiceUUID)->getCharacteristic(dpgCharacteristicUUID);
    dpgChar->registerForNotify(printingNotifyCallback);
    // basic dpg read comand has the same first and last byte, the middle one is the actual command value
    // more info: https://github.com/anson-vandoren/linak-desk-spec/blob/master/dpg_commands.md
    uint8_t data[3] = {0x7f, 0x88, 0x0};

    dpgChar->writeValue(data, 3);
    Serial.println("Dpg response:");
    printStringAsHex(dpgChar->readValue());

    data[1] = 0x89;

    dpgChar->writeValue(data, 3);
    Serial.println("Dpg response:");
    printStringAsHex(dpgChar->readValue());

    data[1] = 0x8a;

    dpgChar->writeValue(data, 3);
    Serial.println("Dpg response:");
    printStringAsHex(dpgChar->readValue());

    data[1] = 0x8b;

    dpgChar->writeValue(data, 3);
    Serial.println("Dpg response:");
    printStringAsHex(dpgChar->readValue());

    data[1] = 0x8c;

    dpgChar->writeValue(data, 3);
    Serial.println("Dpg response:");
    printStringAsHex(dpgChar->readValue());
}

void setup() {
    Serial.begin(115200);

    BLEDevice::init("BLE32");

    client = BLEDevice::createClient();

    // client->setClientCallbacks(new MyClientCallbacks());
    Serial.println("connecting");

    // BLE_ADDR_TYPE_RANDOM is needed, otherwise it won't connect
    auto ret = client->connect(std::string(bt_mac_address), BLE_ADDR_TYPE_RANDOM);
    if (ret) {
        Serial.println("connected");

        auto outputChar = client->getService(outputServiceUUID)->getCharacteristic(outputCharacteristicUUID);
        outputChar->registerForNotify(printingNotifyCallback);

        deskInit();

        Serial.println("Trying the move: ");

        uint8_t moveData[2] = {0xfe, 0x00};
        auto controlChar = client->getService(controlServiceUUID)->getCharacteristic(controlCharacteristicUUID);
        controlChar->writeValue(moveData, 2);

        Serial.println("Control response:");
        printStringAsHex(controlChar->readValue());
        moveData[0] = 0xff;
        controlChar->writeValue(moveData, 2);
        Serial.println("Control response:");
        printStringAsHex(controlChar->readValue());

        uint16_t destination = 700; // raw destination height - 700 is quite low, look out!

        // TODO: this needs a mechanism for breaking out of the loop if the desk hits something
        while (outputChar->readUInt16() != destination) {
            moveData[0] = destination;
            moveData[1] = destination >> 8;
            auto inputChar = client->getService(inputServiceUUID)->getCharacteristic(inputCharacteristicUUID);
            inputChar->writeValue(moveData, 2);
            Serial.println("Input response:");
            printStringAsHex(inputChar->readValue());
            const TickType_t delay = 500 / portTICK_PERIOD_MS;
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