#include "DeskController.h"

#include <chrono>
#include <thread>

#include <Arduino.h>

namespace {
static std::function<void(LinakDesk::HeightSpeedData)> printingCallback = [](LinakDesk::HeightSpeedData data) {
    Serial.print("Notify callback for HeightSpeed: ");
    Serial.print("Height: ");
    Serial.print(data.getRawHeight());
    Serial.print(" Speed: ");
    Serial.println(data.getSpeed());
};
} // namespace

namespace LinakDesk {
DeskController::DeskController(std::unique_ptr<ConnectionInterface> connection) : mConnection(std::move(connection)) {}

DeskController::~DeskController() {}

bool DeskController::connect(std::string bluetoothAddress) {
    if (mConnection->isConnected()) {
        mConnection->disconnect();
    }
    return mConnection->connect(bluetoothAddress);
}
void DeskController::disconnect() { mConnection->disconnect(); }

bool DeskController::isConnected() { return mConnection->isConnected(); }

unsigned short DeskController::getHeight() const { return mConnection->getHeight(); }

bool DeskController::moveToHeight(unsigned short height) const {
    mConnection->attachHeightSpeedCallback(printingCallback);
    mConnection->startMoveTorwards();

    // TODO (#1): this needs a mechanism for breaking out of the loop if the desk hits something
    while (mConnection->getHeight() != height) {
        mConnection->moveTorwards(height);
        delay(200);
    }
    mConnection->stopMove();

    mConnection->attachHeightSpeedCallback({});

    // for now always return true...
    return true;
}

} // namespace LinakDesk
