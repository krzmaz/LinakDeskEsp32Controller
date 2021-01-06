#pragma once

#include <memory>
#include <optional>
#include <string>

#include <BLEDevice.h>

#include "ConnectionInterface.h"

namespace LinakDesk {
class BluetoothConnection : public ConnectionInterface {
  public:
    explicit BluetoothConnection();
    ~BluetoothConnection();
    bool connect(const std::string& bluetoothAddress) override;
    void disconnect() const override;
    bool isConnected() const override;
    unsigned short getHeight() const override;
    void attachHeightSpeedCallback(std::optional<std::function<void(HeightSpeedData)>> callback) const override;
    void startMoveTorwards() const override;
    void moveTorwards(unsigned short height) const override;
    void stopMove() const override;

    // BLE library allows only one callback to be attached, so we might as well make it static
    static std::optional<std::function<void(HeightSpeedData)>> sHeightSpeedCallback;

  private:
    // mimic the calls done by LinakDeskApp after connection
    void setupDesk() const;
    // We need to query the name, otherwise the controller won't react
    void queryName() const;

    void writeUInt16(BLERemoteCharacteristic* charcteristic, unsigned short value) const;

    std::unique_ptr<BLEClient> mBleClient;
    bool mIsConnected = false;

    BLERemoteCharacteristic* mOutputChar;
    BLERemoteCharacteristic* mInputChar;
    BLERemoteCharacteristic* mControlChar;
};

} // namespace LinakDesk
