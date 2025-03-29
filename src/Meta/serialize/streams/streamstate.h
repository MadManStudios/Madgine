#pragma once

#include "Generic/enum.h"

namespace Engine {
namespace Serialize {

    ENUM(StreamState,
        OK,
        UNKNOWN_ERROR,
        PERMISSION_ERROR,
        INTEGRITY_ERROR,
        PARSE_ERROR,
        CLOSED_BY_USER,
        REJECTED,
        CONNECTION_LOST)

}
}