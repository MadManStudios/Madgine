#pragma once

#include "Meta/keyvalue/valuetype.h"

namespace ImGui {

struct ValueTypePayload {
    std::string mName;
    Engine::ValueType mValue;
    mutable std::string mStatusMessage;
};

}
