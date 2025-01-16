#pragma once

#include "Generic/closure.h"

namespace Engine {
namespace Serialize {

    struct SyncFunction {
        void (*mWriteFunctionArguments)(const std::vector<WriteMessage> &, const void *);
        void (*mWriteFunctionResult)(FormattedMessageStream &, const void *);
        StreamResult (*mReadFunctionAction)(SyncableUnitBase *, FormattedMessageStream &, uint16_t, FunctionType, PendingRequest &);
        StreamResult (*mReadFunctionRequest)(SyncableUnitBase *, FormattedMessageStream &, uint16_t, FunctionType, MessageId);
    };

}
}