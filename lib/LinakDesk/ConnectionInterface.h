#pragma once

#include <functional>

#include "HeightSpeedData.h"

namespace LinakDesk {
class ConnectionInterface {
  public:
    virtual ~ConnectionInterface() = default;
    virtual bool isConnected() const = 0;
    virtual unsigned short getHeight() const = 0;
    // virtual void attachHeightSpeedCallback(std::function<void(HeightSpeedData)> callback) = 0;
    virtual void startMoveTorwards() const = 0;
    virtual void moveTorwards(unsigned short height) const = 0;
    virtual void stopMove() const = 0;
};
} // namespace LinakDesk
