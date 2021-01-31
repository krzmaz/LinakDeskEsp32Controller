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
    virtual unsigned short getHeight() const = 0;
    virtual void attachHeightSpeedCallback(const std::function<void(const HeightSpeedData&)>& callback) const = 0;
    virtual void detachHeightSpeedCallback() const = 0;
    virtual void startMoveTorwards() const = 0;
    virtual void moveTorwards(unsigned short height) const = 0;
    virtual void stopMove() const = 0;
};
} // namespace LinakDesk
