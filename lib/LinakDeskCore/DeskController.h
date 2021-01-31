#pragma once

#include <functional>
#include <memory>
#include <string>

#include <Arduino.h>

#include "ConnectionInterface.h"
#include "HeightSpeedData.h"

namespace LinakDesk {

class DeskController {
  public:
    explicit DeskController(std::unique_ptr<ConnectionInterface> connection);
    ~DeskController();

    bool connect(std::string bluetoothAddress);
    void disconnect();
    bool isConnected() const;

    bool moveToHeight(unsigned short destinationHeight) const;
    unsigned short getHeight() const;

    static const std::function<void(const LinakDesk::HeightSpeedData&)> printingCallback;
    static unsigned short sLastHeight;
    static short sLastSpeed;

  private:
    void endMove() const;
    std::unique_ptr<ConnectionInterface> mConnection;
};

} // namespace LinakDesk
