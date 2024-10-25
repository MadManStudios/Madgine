#include "behaviorlib.h"

#include "behaviorreceiver.h"

#include "Meta/keyvalue/valuetype.h"

namespace Engine {

BehaviorError BehaviorReceiver::getBindingHelper(std::string_view name, CallableView<void(const ValueType &)> cb)
{
    ValueType v;
    BehaviorError error = getBinding(name, v);
    if (error.mResult == BehaviorResult { BehaviorResult::SUCCESS })
        cb(v);
    return error;
}

}