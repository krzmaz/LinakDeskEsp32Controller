#include "BluetoothConnection.h"

#include <Arduino.h>

namespace {
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
        mInputChar = mBleClient->getService(BleConstants::InputServiceUUID)
                         ->getCharacteristic(BleConstants::InputCharacteristicUUID);
        mOutputChar = mBleClient->getService(BleConstants::OutputServiceUUID)
                          ->getCharacteristic(BleConstants::OutputCharacteristicUUID);
        mControlChar = mBleClient->getService(BleConstants::ControlServiceUUID)
                           ->getCharacteristic(BleConstants::ControlCharacteristicUUID);
        mDpgChar = mBleClient->getService(BleConstants::DpgServiceUUID)
                       ->getCharacteristic(BleConstants::DpgCharacteristicUUID);
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

void BluetoothConnection::setupDesk() {
    queryName();
    mDpgChar->registerForNotify(printingNotifyCallback);
    // basic dpg read comand has the same first and last byte, the middle one is the actual command value
    // more info: https://github.com/anson-vandoren/linak-desk-spec/blob/master/dpg_commands.md

    auto deskOffsetCommandOutput = dpgReadCommand(DpgCommand::DeskOffset);
    if (deskOffsetCommandOutput[2] == 0x01) {
        mRawOffset = *(unsigned short*)(deskOffsetCommandOutput.c_str() + 3);
    }

    loadMemoryPosition(DpgCommand::MemoryPosition1);
    loadMemoryPosition(DpgCommand::MemoryPosition2);
    loadMemoryPosition(DpgCommand::MemoryPosition3);

    dpgReadCommand(DpgCommand::UserID);

    dpgWriteCommand(DpgCommand::UserID, Constants::UserIdCommandData, 16);

    mDpgChar->registerForNotify(nullptr);
}

void BluetoothConnection::queryName() const {
    const TickType_t delay = 500 / portTICK_PERIOD_MS;
    vTaskDelay(delay); // aparently needed (by the linak controller maybe?)
    auto name = mBleClient->getService(BleConstants::NameServiceUUID)
                    ->getCharacteristic(BleConstants::NameCharacteristicUUID)
                    ->readValue();
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

void BluetoothConnection::attachHeightSpeedCallback(const std::function<void(const HeightSpeedData&)>& callback) const {
    sHeightSpeedCallback = callback;
    mOutputChar->registerForNotify(adapterCallback);
}

void BluetoothConnection::detachHeightSpeedCallback() const {
    sHeightSpeedCallback = {};
    mOutputChar->registerForNotify(nullptr);
}

void BluetoothConnection::stopMove() const {
    writeUInt16(mControlChar, 0xFF);
    writeUInt16(mInputChar, 0x8001);
}

std::string BluetoothConnection::dpgReadCommand(DpgCommand command) {
    unsigned char dataToSend[3]{0x7f, static_cast<unsigned char>(command), 0x0};
    mDpgChar->writeValue(dataToSend, 3);
    return mDpgChar->readValue();
}

std::string BluetoothConnection::dpgWriteCommand(DpgCommand command, const unsigned char* data, unsigned char length) {
    std::vector<unsigned char> dataToSend{0x7f, static_cast<unsigned char>(command), 0x80, 0x01};
    dataToSend.reserve(4 + length);
    std::copy(data, data + length, std::back_inserter(dataToSend));
    mDpgChar->writeValue(dataToSend.data(), dataToSend.size());
    return mDpgChar->readValue();
}

void BluetoothConnection::loadMemoryPosition(DpgCommand command) {
    auto temp = dpgReadCommand(command);
    std::optional<unsigned short> value;
    if (temp[2] == 0x01) {
        value = *(unsigned short*)(temp.c_str() + 3);
    }
    switch (command) {
    case DpgCommand::MemoryPosition1:
        mMemoryPosition1 = value;
        break;
    case DpgCommand::MemoryPosition2:
        mMemoryPosition2 = value;
        break;
    case DpgCommand::MemoryPosition3:
        mMemoryPosition3 = value;
        break;

    default:
        Serial.println("loadMemoryPosition called with wrong command!");
        break;
    }
}

void BluetoothConnection::setMemoryPosition(DpgCommand command, unsigned short value){
    uint8_t data[2];
    data[0] = value;
    data[1] = value >> 8;
    dpgWriteCommand(command, data, 2);
    loadMemoryPosition(command);
}

} // namespace LinakDesk
