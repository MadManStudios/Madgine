#pragma once

#include "Generic/enum.h"

namespace Engine {
namespace Widgets {
    
    ENUM(Formula,
        W_MINUS_H,
        ABS_W_MINUS_H,
        W_OVER_H,
        W,
        H);

    ENUM(Operator,
        GREATER,
        LESS,
        GREATER_OR_EQUAL,
        LESS_OR_EQUAL);
        
    struct Condition {
        std::string mName;

        Formula mFormula;

        Operator mOperator;

        float mReferenceValue = 0.0f;
    };

}
}