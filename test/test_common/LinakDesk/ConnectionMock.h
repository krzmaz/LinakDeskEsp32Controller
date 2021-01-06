#pragma once

#include <gmock/gmock.h>

#include "ConnectionInterface.h"

namespace LinakDesk {
class ConnectionMock : public ConnectionInterface {
  public:
    MOCK_METHOD1(connect, bool(const std::string&));
    MOCK_CONST_METHOD0(disconnect, void());
    MOCK_CONST_METHOD0(isConnected, bool());
    MOCK_CONST_METHOD0(getHeight, unsigned short());
    MOCK_CONST_METHOD1(attachHeightSpeedCallback, void(std::optional<std::function<void(HeightSpeedData)>>));
    MOCK_CONST_METHOD0(startMoveTorwards, void());
    MOCK_CONST_METHOD1(moveTorwards, void(unsigned short height));
    MOCK_CONST_METHOD0(stopMove, void());
};

} // namespace LinakDesk
