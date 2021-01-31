#include "DeskController.h"

#include <Arduino.h>
#include <Serial.h>
#include <gmock/gmock.h>

#include "ConnectionMock.h"

using namespace testing;
using namespace LinakDesk;

class DeskControllerTest : public Test {
  public:
    DeskControllerTest() : DeskControllerTest(std::make_unique<StrictMock<ConnectionMock>>()) {}

    DeskControllerTest(std::unique_ptr<StrictMock<ConnectionMock>> connection)
        : mConnectionMock(*connection), mDeskController(std::move(connection)) {}

    void setPrintingCallbackCallsExpecations(const HeightSpeedData& data) {
        EXPECT_CALL(*mSerialMock, print(Matcher<const char*>(StrEq("Notify callback for HeightSpeed: "))));
        EXPECT_CALL(*mSerialMock, print(Matcher<const char*>(StrEq("Height: "))));
        EXPECT_CALL(*mSerialMock, print(TypedEq<int>(data.getRawHeight()), 10));
        EXPECT_CALL(*mSerialMock, print(Matcher<const char*>(StrEq(" Speed: "))));
        EXPECT_CALL(*mSerialMock, println(TypedEq<int>(data.getSpeed()), 10));
    }

    StrictMock<ConnectionMock>& mConnectionMock;
    DeskController mDeskController;
    InSequence mSequence; // ensure the mock calls are in order

    const std::function<void(const HeightSpeedData&)>* mHeightSpeedCallbackPtr;

    // custom deleters to avoid custom destructor
    std::unique_ptr<SerialMock, std::function<void(SerialMock*)>> mSerialMock{
        serialMockInstance(), [](SerialMock*) { releaseSerialMock(); }}; // TODO: Change to StrictMock
    std::unique_ptr<ArduinoMock, std::function<void(ArduinoMock*)>> mArduinoMock{
        arduinoMockInstance(), [](ArduinoMock*) { releaseArduinoMock(); }}; // TODO: Change to StrictMock
};

TEST_F(DeskControllerTest, connectingWorksIfNotConnected) {
    std::string address = "some:Bt:Address";
    EXPECT_CALL(mConnectionMock, isConnected()).WillOnce(Return(false));
    EXPECT_CALL(mConnectionMock, connect(address));
    mDeskController.connect(address);
}
TEST_F(DeskControllerTest, connectingDisconnectsFirstIfConnected) {
    std::string address = "some:Bt:Address";
    EXPECT_CALL(mConnectionMock, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(mConnectionMock, disconnect());
    EXPECT_CALL(mConnectionMock, connect(address));
    mDeskController.connect(address);
}

TEST_F(DeskControllerTest, gettingConnectionStatusForwardsCalls) {
    EXPECT_CALL(mConnectionMock, isConnected()).WillOnce(Return(true));
    EXPECT_EQ(mDeskController.isConnected(), true);
    EXPECT_CALL(mConnectionMock, isConnected()).WillOnce(Return(false));
    EXPECT_EQ(mDeskController.isConnected(), false);
}

TEST_F(DeskControllerTest, movingToHeightReturnsFalseIfWereDisconnected) {
    unsigned short destinationHeight = 1234u;
    EXPECT_CALL(mConnectionMock, isConnected()).WillOnce(Return(false));
    EXPECT_FALSE(mDeskController.moveToHeight(destinationHeight));
}

TEST_F(DeskControllerTest, movingToHeightReturnsTrueIfWereAtPositionAlready) {
    unsigned short destinationHeight = 1234u;
    EXPECT_CALL(mConnectionMock, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(mConnectionMock, getHeight()).WillOnce(Return(destinationHeight));
    EXPECT_TRUE(mDeskController.moveToHeight(destinationHeight));
}

TEST_F(DeskControllerTest, movingToHeightReturnsFalseIfPositionDoesntChange) {
    unsigned short destinationHeight = 1234u;
    unsigned short startHeight = 1234u - 50u;

    EXPECT_CALL(mConnectionMock, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(mConnectionMock, getHeight()).WillOnce(Return(startHeight));
    EXPECT_CALL(mConnectionMock, attachHeightSpeedCallback(_))
        .WillOnce(
            [&](const std::function<void(const HeightSpeedData&)>& callback) { mHeightSpeedCallbackPtr = &callback; });
    EXPECT_CALL(mConnectionMock, startMoveTorwards());
    EXPECT_CALL(mConnectionMock, moveTorwards(destinationHeight));
    EXPECT_CALL(*mArduinoMock, delay(200));
    EXPECT_CALL(mConnectionMock, stopMove());
    EXPECT_CALL(mConnectionMock, detachHeightSpeedCallback());

    EXPECT_FALSE(mDeskController.moveToHeight(destinationHeight));
}

TEST_F(DeskControllerTest, basicMoveToHeightScenarioGoingUp) {
    unsigned short destinationHeight = 1234u;
    unsigned short startHeight = destinationHeight - 100u;
    auto data1 = HeightSpeedData(destinationHeight - 50, 50);
    auto data2 = HeightSpeedData(destinationHeight, 0);
    EXPECT_CALL(mConnectionMock, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(mConnectionMock, getHeight()).WillOnce(Return(startHeight));
    EXPECT_CALL(mConnectionMock, attachHeightSpeedCallback(_))
        .WillOnce(
            [&](const std::function<void(const HeightSpeedData&)>& callback) { mHeightSpeedCallbackPtr = &callback; });
    EXPECT_CALL(mConnectionMock, startMoveTorwards());
    EXPECT_CALL(mConnectionMock, moveTorwards(destinationHeight)).WillOnce([&](unsigned short) {
        mHeightSpeedCallbackPtr->operator()(data1);
        mHeightSpeedCallbackPtr->operator()(data2);
    });
    setPrintingCallbackCallsExpecations(data1);
    setPrintingCallbackCallsExpecations(data2);
    EXPECT_CALL(*mArduinoMock, delay(200));
    EXPECT_CALL(mConnectionMock, stopMove());
    EXPECT_CALL(mConnectionMock, detachHeightSpeedCallback());

    EXPECT_TRUE(mDeskController.moveToHeight(destinationHeight));
}

TEST_F(DeskControllerTest, basicMoveToHeightScenarioGoingDown) {
    unsigned short destinationHeight = 1234u;
    unsigned short startHeight = destinationHeight + 100;
    auto data1 = HeightSpeedData(destinationHeight + 50, -50);
    auto data2 = HeightSpeedData(destinationHeight, 0);
    EXPECT_CALL(mConnectionMock, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(mConnectionMock, getHeight()).WillOnce(Return(startHeight));
    EXPECT_CALL(mConnectionMock, attachHeightSpeedCallback(_))
        .WillOnce(
            [&](const std::function<void(const HeightSpeedData&)>& callback) { mHeightSpeedCallbackPtr = &callback; });
    EXPECT_CALL(mConnectionMock, startMoveTorwards());
    EXPECT_CALL(mConnectionMock, moveTorwards(destinationHeight)).WillOnce([&](unsigned short) {
        mHeightSpeedCallbackPtr->operator()(data1);
        mHeightSpeedCallbackPtr->operator()(data2);
    });
    setPrintingCallbackCallsExpecations(data1);
    setPrintingCallbackCallsExpecations(data2);
    EXPECT_CALL(*mArduinoMock, delay(200));
    EXPECT_CALL(mConnectionMock, stopMove());
    EXPECT_CALL(mConnectionMock, detachHeightSpeedCallback());

    EXPECT_TRUE(mDeskController.moveToHeight(destinationHeight));
}

TEST_F(DeskControllerTest, collisionScenarioGoingDown) {
    unsigned short destinationHeight = 1234u;
    unsigned short startHeight = destinationHeight + 500;
    auto data1 = HeightSpeedData(destinationHeight + 400, -50);
    auto data2 = HeightSpeedData(destinationHeight + 300, -50);
    auto data3 = HeightSpeedData(destinationHeight + 200, -50);
    auto data4 = HeightSpeedData(destinationHeight + 300, 50);
    auto data5 = HeightSpeedData(destinationHeight + 400, 0);

    EXPECT_CALL(mConnectionMock, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(mConnectionMock, getHeight()).WillOnce(Return(startHeight));
    EXPECT_CALL(mConnectionMock, attachHeightSpeedCallback(_))
        .WillOnce(
            [&](const std::function<void(const HeightSpeedData&)>& callback) { mHeightSpeedCallbackPtr = &callback; });
    EXPECT_CALL(mConnectionMock, startMoveTorwards());
    EXPECT_CALL(mConnectionMock, moveTorwards(destinationHeight)).WillOnce([&](unsigned short) {
        mHeightSpeedCallbackPtr->operator()(data1);
        mHeightSpeedCallbackPtr->operator()(data2);
    });
    setPrintingCallbackCallsExpecations(data1);
    setPrintingCallbackCallsExpecations(data2);
    EXPECT_CALL(*mArduinoMock, delay(200));
    EXPECT_CALL(mConnectionMock, moveTorwards(destinationHeight)).WillOnce([&](unsigned short) {
        mHeightSpeedCallbackPtr->operator()(data3);
        mHeightSpeedCallbackPtr->operator()(data4);
        mHeightSpeedCallbackPtr->operator()(data5);
    });
    setPrintingCallbackCallsExpecations(data3);
    setPrintingCallbackCallsExpecations(data4);
    setPrintingCallbackCallsExpecations(data5);
    EXPECT_CALL(*mArduinoMock, delay(200));
    EXPECT_CALL(mConnectionMock, stopMove());
    EXPECT_CALL(mConnectionMock, detachHeightSpeedCallback());

    EXPECT_FALSE(mDeskController.moveToHeight(destinationHeight));
}

TEST_F(DeskControllerTest, gettingHeightWorks) {
    unsigned short height = 123;
    EXPECT_CALL(mConnectionMock, getHeight()).WillOnce(Return(height));
    EXPECT_EQ(mDeskController.getHeight(), height);
}