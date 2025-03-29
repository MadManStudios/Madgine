#include "behaviorlib.h"

#include "bindings.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"
#include "Meta/keyvalue/valuetype.h"
#include "Meta/keyvalueutil/valuetypeserialize.h"

METATABLE_BEGIN(Engine::BindingDescriptor)
MEMBER(mName)
MEMBER(mType)
METATABLE_END(Engine::BindingDescriptor)

SERIALIZETABLE_BEGIN(Engine::BindingDescriptor)
FIELD(mName)
FIELD(mType)
SERIALIZETABLE_END(Engine::BindingDescriptor)

namespace Engine {

BehaviorError get_binding_d_t::type_erased(CallableView<BehaviorError(ValueType &)> cb)
{
    ValueType v;
    return cb(v);
}

}