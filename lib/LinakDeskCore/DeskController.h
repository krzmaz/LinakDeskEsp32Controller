#pragma once

#include <memory>
#include <string>

#include "ConnectionInterface.h"

namespace LinakDesk {

class DeskController {
  public:
    explicit DeskController(std::unique_ptr<ConnectionInterface> connection);
    ~DeskController();

    bool connect(std::string bluetoothAddress);
    void disconnect();
    bool isConnected();

    bool moveToHeight(unsigned short height) const;
    unsigned short getHeight() const;

  private:
    std::unique_ptr<ConnectionInterface> mConnection;
};

} // namespace LinakDesk
