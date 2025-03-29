#pragma once

#include "Generic/enum.h"

namespace Engine {
namespace Log {

    ENUM(MessageType,
        DEBUG_TYPE,
        INFO_TYPE,
        WARNING_TYPE,
        ERROR_TYPE,
        FATAL_TYPE
    )

}
}
