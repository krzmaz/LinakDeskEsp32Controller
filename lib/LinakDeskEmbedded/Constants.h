#pragma once

#include <NimBLEUUID.h>

namespace LinakDesk {

namespace Constants {
    static constexpr unsigned char UserIdCommandData[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                                          0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
} // namespace Constants

namespace BleConstants {
    static const BLEUUID NameServiceUUID((unsigned short)0x1800);
    static const BLEUUID NameCharacteristicUUID((unsigned short)0x2a00u);
    static const BLEUUID ControlServiceUUID(0x99fa0001, 0x338a, 0x1024, 0x8a49009c0215f78a);
    static const BLEUUID ControlCharacteristicUUID(0x99fa0002, 0x338a, 0x1024, 0x8a49009c0215f78a);
    static const BLEUUID DpgServiceUUID(0x99fa0010, 0x338a, 0x1024, 0x8a49009c0215f78a);
    static const BLEUUID DpgCharacteristicUUID(0x99fa0011, 0x338a, 0x1024, 0x8a49009c0215f78a);
    static const BLEUUID OutputServiceUUID(0x99fa0020, 0x338a, 0x1024, 0x8a49009c0215f78a);
    static const BLEUUID OutputCharacteristicUUID(0x99fa0021, 0x338a, 0x1024, 0x8a49009c0215f78a);
    static const BLEUUID InputServiceUUID(0x99fa0030, 0x338a, 0x1024, 0x8a49009c0215f78a);
    static const BLEUUID InputCharacteristicUUID(0x99fa0031, 0x338a, 0x1024, 0x8a49009c0215f78a);
} // namespace BleConstants

enum class DpgCommand {
    Capabilities = 0x80,
    UserID = 0x86,
    DeskOffset = 0x81,
    ReminderSetting = 0x88,
    MemoryPosition1 = 0x89,
    MemoryPosition2 = 0x8a,
    MemoryPosition3 = 0x8b,
    MemoryPosition4 = 0x8c
};

} // namespace LinakDesk
