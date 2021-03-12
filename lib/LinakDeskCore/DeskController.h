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

    bool moveToHeight(unsigned short destinationHeight);
    unsigned short getHeight() const;

    void loop();

    static const std::function<void(const LinakDesk::HeightSpeedData&)> printingCallback;
    static unsigned short sLastHeight;
    static short sLastSpeed;

  private:
    void endMove();
    std::unique_ptr<ConnectionInterface> mConnection;
    bool mIsMoving = false;
    bool mGoingUp = false;
    unsigned short mDestinationHeight = 0;
    unsigned short mMoveStartHeight = 0;
    unsigned short mPreviousHeight = 0;
    unsigned long mLastCommandSendTime = 0;
};

} // namespace LinakDesk
