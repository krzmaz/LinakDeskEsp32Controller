#pragma once

#include <memory>
#include <optional>
#include <string>

#include <NimBLEDevice.h>

#include "ConnectionInterface.h"
#include "Constants.h"

namespace LinakDesk {
class BluetoothConnection : public ConnectionInterface {
  public:
    explicit BluetoothConnection();
    ~BluetoothConnection();
    bool connect(const std::string& bluetoothAddress) override;
    void disconnect() const override;
    bool isConnected() const override;
    unsigned short getHeightRaw() const override;
    unsigned short getHeightMm() const override;
    void attachHeightSpeedCallback(const std::function<void(const HeightSpeedData&)>& callback) const override;
    void detachHeightSpeedCallback() const override;
    void startMoveTorwards() const override;
    void moveTorwards(unsigned short height) const override;
    void stopMove() const override;
    const std::optional<unsigned short>& getMemoryPosition(unsigned char positionNumber) const override;
    bool setMemoryPosition(unsigned char positionNumber, unsigned short value) override;
    const std::optional<unsigned short>& getDeskOffset() const override;

    // BLE library allows only one callback to be attached, so we might as well make it static
    static std::optional<std::function<void(HeightSpeedData)>> sHeightSpeedCallback;

  private:
    // mimic the calls done by LinakDeskApp after connection
    void setupDesk();
    // We need to query the name, otherwise the controller won't react
    void queryName() const;

    std::string dpgReadCommand(DpgCommand command);
    std::string dpgWriteCommand(DpgCommand command, const unsigned char* data, unsigned char length);

    void loadMemoryPosition(DpgCommand command);
    void setMemoryPosition(DpgCommand command, unsigned short value);

    void writeUInt16(BLERemoteCharacteristic* charcteristic, unsigned short value) const;

    std::unique_ptr<BLEClient, void (*)(BLEClient*)> mBleClient;
    bool mIsConnected = false;
    std::optional<unsigned short> mRawOffset;
    std::optional<unsigned short> mMemoryPosition1;
    std::optional<unsigned short> mMemoryPosition2;
    std::optional<unsigned short> mMemoryPosition3;

    BLERemoteCharacteristic* mOutputChar = nullptr;
    BLERemoteCharacteristic* mInputChar = nullptr;
    BLERemoteCharacteristic* mControlChar = nullptr;
    BLERemoteCharacteristic* mDpgChar = nullptr;
};

} // namespace LinakDesk
