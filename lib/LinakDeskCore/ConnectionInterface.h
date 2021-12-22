#pragma once

#include <functional>
#include <optional>
#include <string>

#include "HeightSpeedData.h"

namespace LinakDesk {
class ConnectionInterface {
  public:
    virtual ~ConnectionInterface() = default;
    virtual bool connect(const std::string& bluetoothAddress) = 0;
    virtual void disconnect() const = 0;
    virtual bool isConnected() const = 0;
    virtual unsigned short getHeightRaw() const = 0;
    virtual unsigned short getHeightMm() const = 0;
    virtual void attachHeightSpeedCallback(const std::function<void(const HeightSpeedData&)>& callback) const = 0;
    virtual void detachHeightSpeedCallback() const = 0;
    virtual void startMoveTorwards() const = 0;
    virtual void moveTorwards(unsigned short height) const = 0;
    virtual void stopMove() const = 0;
    virtual const std::optional<unsigned short>& getMemoryPosition(unsigned char positionNumber) const = 0;
    virtual bool setMemoryPosition(unsigned char positionNumber, unsigned short value) = 0;
    virtual const std::optional<unsigned short>& getDeskOffset() const = 0;
};
} // namespace LinakDesk
