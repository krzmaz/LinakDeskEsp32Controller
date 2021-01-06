#include "BluetoothConnection.h"

#include <Arduino.h>

namespace {
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

void adapterCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    if (LinakDesk::BluetoothConnection::sHeightSpeedCallback && length >= 4) {
        uint16_t height = *(uint16_t*)pData;
        short speed = *(uint16_t*)(pData + 2);
        LinakDesk::HeightSpeedData hsData(height, speed);
        LinakDesk::BluetoothConnection::sHeightSpeedCallback->operator()(hsData);
    }
}

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
} // namespace

namespace LinakDesk {

std::optional<std::function<void(HeightSpeedData)>> BluetoothConnection::sHeightSpeedCallback = {};

BluetoothConnection::BluetoothConnection() : mBleClient{BLEDevice::createClient()} {}

BluetoothConnection::~BluetoothConnection() {
    disconnect();
    if (BLEDevice::getInitialized()) {
        BLEDevice::deinit();
    }
}

bool BluetoothConnection::connect(const std::string& bluetoothAddress) {
    if (!BLEDevice::getInitialized()) {
        BLEDevice::init("BLE32");
    }
    auto connected = mBleClient->connect(bluetoothAddress, BLE_ADDR_TYPE_RANDOM);
    if (connected) {
        mInputChar = mBleClient->getService(inputServiceUUID)->getCharacteristic(inputCharacteristicUUID);
        mOutputChar = mBleClient->getService(outputServiceUUID)->getCharacteristic(outputCharacteristicUUID);
        mControlChar = mBleClient->getService(controlServiceUUID)->getCharacteristic(controlCharacteristicUUID);
        setupDesk();
        Serial.println("connected");
    }
    return connected;
}

void BluetoothConnection::disconnect() const {
    if (mBleClient->isConnected()) {
        mBleClient->disconnect();
    }
}

bool BluetoothConnection::isConnected() const { return mBleClient->isConnected(); }

void BluetoothConnection::writeUInt16(BLERemoteCharacteristic* charcteristic, unsigned short value) const {
    uint8_t data[2];
    data[0] = value;
    data[1] = value >> 8;
    charcteristic->writeValue(data, 2);
}

void BluetoothConnection::setupDesk() const {
    queryName();
    auto dpgChar = mBleClient->getService(dpgServiceUUID)->getCharacteristic(dpgCharacteristicUUID);
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

void BluetoothConnection::queryName() const {
    const TickType_t delay = 500 / portTICK_PERIOD_MS;
    vTaskDelay(delay); // aparently needed (by the linak controller maybe?)
    auto name = mBleClient->getService(nameServiceUUID)->getCharacteristic(nameCharacteristicUUID)->readValue();
    vTaskDelay(delay); // aparently needed (by the linak controller maybe?)
    Serial.println("Name:");
    Serial.println(name.c_str());
}

unsigned short BluetoothConnection::getHeight() const { return mOutputChar->readUInt16(); }

void BluetoothConnection::startMoveTorwards() const {
    writeUInt16(mControlChar, 0xFE);
    Serial.println("Control response:");
    printStringAsHex(mControlChar->readValue());

    writeUInt16(mControlChar, 0xFF);
    Serial.println("Control response:");
    printStringAsHex(mControlChar->readValue());
}
void BluetoothConnection::moveTorwards(unsigned short height) const {
    writeUInt16(mInputChar, height);
    Serial.println("Input response:");
    printStringAsHex(mInputChar->readValue());
}

void BluetoothConnection::attachHeightSpeedCallback(std::optional<std::function<void(HeightSpeedData)>> callback) const {
    if(callback){
        sHeightSpeedCallback = callback;
        mOutputChar->registerForNotify(adapterCallback);
    }
    else{
        // deregister
        sHeightSpeedCallback = {};
        mOutputChar->registerForNotify(nullptr);
    }
}

void BluetoothConnection::stopMove() const {
    writeUInt16(mControlChar, 0xFF);
    writeUInt16(mInputChar, 0x8001);
}
} // namespace LinakDesk
