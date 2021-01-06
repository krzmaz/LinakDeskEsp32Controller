#pragma once

#include "BluetoothConnection.h"
#include "DeskController.h"

namespace LinakDesk {
class DeskControllerFactory {
  public:
    DeskControllerFactory();
    virtual ~DeskControllerFactory() = 0;
    static DeskController make() {
        return DeskController(std::make_unique<BluetoothConnection>());
    };
};
} // namespace LinakDesk
