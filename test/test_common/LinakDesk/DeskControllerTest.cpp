#include "DeskController.h"

#include <gtest/gtest.h>

#include "ConnectionMock.h"

using namespace testing;
using namespace LinakDesk;

class DeskControllerTest : public Test {
  public:
    DeskControllerTest() : DeskControllerTest(std::make_unique<StrictMock<ConnectionMock>>()) {}

    DeskControllerTest(std::unique_ptr<StrictMock<ConnectionMock>> connection)
        : mConnectionMock(*connection), mDeskController(std::move(connection)) {}
    StrictMock<ConnectionMock>& mConnectionMock;
    DeskController mDeskController;
};

TEST_F(DeskControllerTest, gettingHeightWorks) {
    unsigned short height = 123;
    EXPECT_CALL(mConnectionMock, getHeight()).WillOnce(Return(height));
    EXPECT_EQ(mDeskController.getHeight(), height);
}