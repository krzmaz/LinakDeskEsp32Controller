#pragma once

#include <BLEUUID.h>

namespace LinakDesk {

namespace Constants {
    static constexpr unsigned char UserIdCommandData[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                                          0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
} // namespace Constants

namespace BleConstants {
    static const BLEUUID NameServiceUUID("00001800-0000-1000-8000-00805f9b34fb");
    static const BLEUUID NameCharacteristicUUID("00002a00-0000-1000-8000-00805f9b34fb");
    static const BLEUUID ControlServiceUUID("99fa0001-338a-1024-8a49-009c0215f78a");
    static const BLEUUID ControlCharacteristicUUID("99fa0002-338a-1024-8a49-009c0215f78a");
    static const BLEUUID DpgServiceUUID("99fa0010-338a-1024-8a49-009c0215f78a");
    static const BLEUUID DpgCharacteristicUUID("99fa0011-338a-1024-8a49-009c0215f78a");
    static const BLEUUID OutputServiceUUID("99fa0020-338a-1024-8a49-009c0215f78a");
    static const BLEUUID OutputCharacteristicUUID("99fa0021-338a-1024-8a49-009c0215f78a");
    static const BLEUUID InputServiceUUID("99fa0030-338a-1024-8a49-009c0215f78a");
    static const BLEUUID InputCharacteristicUUID("99fa0031-338a-1024-8a49-009c0215f78a");
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
