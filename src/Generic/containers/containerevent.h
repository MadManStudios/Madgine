#pragma once

namespace Engine {
namespace Containers {

    enum ContainerEvent {
        EMPLACE = 0x1,
        ERASE = 0x2,
        ERASE_RANGE = 0x3,
        SWAP_ERASE = 0x4,
        RESET = 0x5,

        ACTIVATE_ITEM = 0x6,
        DEACTIVATE_ITEM = 0x7,

        MASK = 0x0F,

        BEFORE = 0x00,
        AFTER = 0x10,
        ABORTED = 0x20,

        FULL_MASK = 0x3F
    };

}
}