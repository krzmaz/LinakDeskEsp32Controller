#include "DeskController.h"

#include <chrono>
#include <cmath>
#include <limits>

#include <Arduino.h>

namespace LinakDesk {

unsigned short DeskController::sLastHeight = std::numeric_limits<unsigned short>::max();

short DeskController::sLastSpeed = std::numeric_limits<short>::max();

DeskController::DeskController(std::unique_ptr<ConnectionInterface> connection) : mConnection(std::move(connection)) {}

DeskController::~DeskController() {}

bool DeskController::connect(std::string bluetoothAddress) {
    if (mConnection->isConnected()) {
        mConnection->disconnect();
    }
    return mConnection->connect(bluetoothAddress);
}
void DeskController::disconnect() { mConnection->disconnect(); }

bool DeskController::isConnected() const { return mConnection->isConnected(); }

unsigned short DeskController::getHeight() const { return mConnection->getHeight(); }

bool DeskController::moveToHeight(unsigned short destinationHeight) const {
    if (!isConnected()) {
        return false;
    }
    auto startHeight = mConnection->getHeight();
    if (startHeight == destinationHeight) {
        return true;
    }
    auto goingUp = std::signbit(startHeight - destinationHeight);

    sLastHeight = std::numeric_limits<unsigned short>::max();
    sLastSpeed = std::numeric_limits<short>::max();

    mConnection->attachHeightSpeedCallback(printingCallback);
    mConnection->startMoveTorwards();

    mConnection->moveTorwards(destinationHeight);
    delay(200);
    if (sLastHeight == std::numeric_limits<unsigned short>::max()) {
        // For some reason the callback wasn't called
        endMove();
        return false;
    }

    auto previousHeight = sLastHeight;

    while (sLastHeight != destinationHeight) {
        if (sLastSpeed == 0) {
            // We've stopped at the wrong height, something's wrong
            endMove();
            return false;
        }
        if (!std::signbit(sLastSpeed) != goingUp) {
            // We're moving in the wrong direction, something's wrong
            endMove();
            return false;
        }

        mConnection->moveTorwards(destinationHeight);
        delay(200);
        if (previousHeight == sLastHeight) {
            // We didn't move in the last 200ms, something's wrong
            endMove();
            return false;
        }
    }
    // We reached our destination
    endMove();

    return true;
}

void DeskController::endMove() const {
    mConnection->stopMove();
    mConnection->detachHeightSpeedCallback();
}

const std::function<void(const HeightSpeedData&)> DeskController::printingCallback = [](const HeightSpeedData& data) {
    Serial.print("Notify callback for HeightSpeed: ");
    Serial.print("Height: ");
    sLastHeight = data.getRawHeight();
    Serial.print(sLastHeight);
    Serial.print(" Speed: ");
    sLastSpeed = data.getSpeed();
    Serial.println(sLastSpeed);
};

} // namespace LinakDesk