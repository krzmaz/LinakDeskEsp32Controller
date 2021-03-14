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

unsigned short DeskController::getHeightRaw() const { return mConnection->getHeightRaw(); }
unsigned short DeskController::getHeightMm() const { return mConnection->getHeightMm(); }

const std::optional<unsigned short>& DeskController::getMemoryPosition(unsigned char positionNumber) const{
    return mConnection->getMemoryPosition(positionNumber);
}

bool DeskController::moveToHeightMm(unsigned short destinationHeight) {
    auto offset = mConnection->getDeskOffset();
    if(offset){
        return moveToHeightRaw(destinationHeight*10 - offset.value());
    }
    return false;
}

bool DeskController::moveToHeightRaw(unsigned short destinationHeight) {
    if (!isConnected() || mIsMoving) {
        return false;
    }
    mMoveStartHeight = mConnection->getHeightRaw();
    if (mMoveStartHeight == destinationHeight) {
        return true;
    }
    mGoingUp = std::signbit(mMoveStartHeight - destinationHeight);
    mDestinationHeight = destinationHeight;
    sLastHeight = std::numeric_limits<unsigned short>::max();
    sLastSpeed = std::numeric_limits<short>::max();

    mConnection->attachHeightSpeedCallback(printingCallback);
    mConnection->startMoveTorwards();

    mConnection->moveTorwards(destinationHeight);
    mLastCommandSendTime = millis();
    mIsMoving = true;

    return true;
}

void DeskController::loop() {
    if (!mIsMoving) {
        return;
    }
    if (millis() - mLastCommandSendTime > 150) {
        if (sLastHeight == std::numeric_limits<unsigned short>::max()) {
            // For some reason the callback wasn't called
            endMove();
            return;
        }

        if (mPreviousHeight == sLastHeight) {
            // We didn't move since the last time, something's wrong
            endMove();
            return;
        }

        if (sLastHeight != mDestinationHeight) {
            if (sLastSpeed == 0) {
                // We've stopped at the wrong height, something's wrong
                endMove();
                return;
            }
            if (!std::signbit(sLastSpeed) != mGoingUp) {
                // We're moving in the wrong direction, something's wrong
                endMove();
                return;
            }

            mPreviousHeight = sLastHeight;
            mConnection->moveTorwards(mDestinationHeight);
            mLastCommandSendTime = millis();
            return;
        }
        // We reached our destination
        endMove();
    }
}

void DeskController::endMove() {
    mConnection->stopMove();
    mConnection->detachHeightSpeedCallback();
    mIsMoving = false;
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
