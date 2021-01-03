#include "BluetoothConnection.h"

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
// void adapterCallback (BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData,
//                                              size_t length, bool isNotify){
//     if (LinakDesk::BluetoothConnection::mCallback && length >=4){
//         uint16_t height = *(uint16_t*)pData;
//         short speed = *(uint16_t*)pData+2;
//         LinakDesk::HeightSpeedData hsData(height, speed);
//         LinakDesk::BluetoothConnection::mCallback->operator()(hsData);
//     }
//                                              }
} // namespace

namespace LinakDesk {
BluetoothConnection::BluetoothConnection(const std::string& bluetoothAddress)
    : mBleClient{BLEDevice::createClient()} {
    BLEDevice::init("BLE32");
    mIsConnected = mBleClient->connect(bluetoothAddress, BLE_ADDR_TYPE_RANDOM);
    if (mIsConnected) {
        mInputChar = mBleClient->getService(inputServiceUUID)->getCharacteristic(inputCharacteristicUUID);
        mOutputChar = mBleClient->getService(outputServiceUUID)->getCharacteristic(outputCharacteristicUUID);
        mControlChar = mBleClient->getService(controlServiceUUID)->getCharacteristic(controlCharacteristicUUID);
        setupDesk();
        // Serial.println("connected");
    }
}

BluetoothConnection::~BluetoothConnection(){
    if(BLEDevice::getInitialized()){
        mBleClient->disconnect();
        // BLEDevice::deinit(false);
    }
}

void BluetoothConnection::writeUInt16(BLERemoteCharacteristic* charcteristic, unsigned short value) const{
    uint8_t data[2];
    data[0] = value;
    data[1] = value >> 8;
    charcteristic->writeValue(data, 2);
}

void BluetoothConnection::setupDesk() const {
    queryName();
    auto dpgChar = mBleClient->getService(dpgServiceUUID)->getCharacteristic(dpgCharacteristicUUID);
    // dpgChar->registerForNotify(printingNotifyCallback);
    // basic dpg read comand has the same first and last byte, the middle one is the actual command value
    // more info: https://github.com/anson-vandoren/linak-desk-spec/blob/master/dpg_commands.md
    uint8_t data[3] = {0x7f, 0x88, 0x0};

    dpgChar->writeValue(data, 3);
    // Serial.println("Dpg response:");
    // printStringAsHex(dpgChar->readValue());

    data[1] = 0x89;

    dpgChar->writeValue(data, 3);
    // Serial.println("Dpg response:");
    // printStringAsHex(dpgChar->readValue());

    data[1] = 0x8a;

    dpgChar->writeValue(data, 3);
    // Serial.println("Dpg response:");
    // printStringAsHex(dpgChar->readValue());

    data[1] = 0x8b;

    dpgChar->writeValue(data, 3);
    // Serial.println("Dpg response:");
    // printStringAsHex(dpgChar->readValue());

    data[1] = 0x8c;

    dpgChar->writeValue(data, 3);
    // Serial.println("Dpg response:");
    // printStringAsHex(dpgChar->readValue());
}

void BluetoothConnection::queryName() const {
    const TickType_t delay = 500 / portTICK_PERIOD_MS;
    vTaskDelay(delay); // aparently needed (by the linak controller maybe?)
    auto name = mBleClient->getService(nameServiceUUID)->getCharacteristic(nameCharacteristicUUID)->readValue();
    vTaskDelay(delay); // aparently needed (by the linak controller maybe?)
    // Serial.println("Name:");
    // Serial.println(name.c_str());
}

bool BluetoothConnection::isConnected() const {
    return mIsConnected;
}
unsigned short BluetoothConnection::getHeight() const {
    return mOutputChar->readUInt16();
}
void BluetoothConnection::startMoveTorwards() const {
    // uint8_t moveData[2] = {0xfe, 0x00};
    // mControlChar->writeValue(moveData, 2);
    writeUInt16(mControlChar,0xFE);

    // Serial.println("Control response:");
    // printStringAsHex(controlChar->readValue());
    // moveData[0] = 0xff;
    // mControlChar->writeValue(moveData, 2);
    writeUInt16(mControlChar,0xFF);
    // Serial.println("Control response:");
    // printStringAsHex(controlChar->readValue());
    
}
void BluetoothConnection::moveTorwards(unsigned short height) const {
    // uint8_t moveData[2];
    // moveData[0] = height;
    // moveData[1] = height >> 8;
    // mInputChar->writeValue(moveData, 2);
    writeUInt16(mInputChar, height);
    // Serial.println("Input response:");
    // printStringAsHex(inputChar->readValue());
}

// void BluetoothConnection::adapterCallback (BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData,
//                                              size_t length, bool isNotify){
//     if (mCallback && length >=4){
//         uint16_t height = *(uint16_t*)pData;
//         short speed = *(uint16_t*)pData+2;
//         LinakDesk::HeightSpeedData hsData(height, speed);
//         mCallback->operator()(hsData);
//     }
// }
// void BluetoothConnection::attachHeightSpeedCallback(std::function<void(HeightSpeedData)> callback) {
//     mCallback = callback;
//     mOutputChar->registerForNotify(adapterCallback);

// }
void BluetoothConnection::stopMove() const {
    writeUInt16(mControlChar,0xFF);
    writeUInt16(mInputChar,0x8001);
}
} // namespace LinakDesk
