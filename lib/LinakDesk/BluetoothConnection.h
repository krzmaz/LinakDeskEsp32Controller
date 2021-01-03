#pragma once

#include <BLEDevice.h>
#include <optional>
#include <memory>
#include <string>

#include "ConnectionInterface.h"

namespace LinakDesk {
class BluetoothConnection : public ConnectionInterface {
  public:
    explicit BluetoothConnection(const std::string& bluetoothAddress);
    ~BluetoothConnection();
    bool isConnected() const override;
    unsigned short getHeight() const override;
    // void attachHeightSpeedCallback(std::function<void(HeightSpeedData)> callback) override;
    void startMoveTorwards() const override;
    void moveTorwards(unsigned short height) const override;
    void stopMove() const override;

    // static std::optional<std::function<void(HeightSpeedData)>> mCallback;
  private:
    // mimic the calls done by LinakDeskApp after connection
    void setupDesk() const;
    // We need to query the name, otherwise the controller won't react
    void queryName() const;

    void writeUInt16(BLERemoteCharacteristic* charcteristic, unsigned short value) const;

    // void adapterCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData,
    //                                          size_t length, bool isNotify);
    std::unique_ptr<BLEClient> mBleClient;
    bool mIsConnected = false;

    BLERemoteCharacteristic* mOutputChar;
    BLERemoteCharacteristic* mInputChar;
    BLERemoteCharacteristic* mControlChar;
};

} // namespace LinakDesk
