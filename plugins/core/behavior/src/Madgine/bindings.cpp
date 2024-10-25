#include "behaviorlib.h"

#include "bindings.h"

#include "Meta/keyvalue/metatable_impl.h"

METATABLE_BEGIN(Engine::BindingDescriptor)
MEMBER(mName)
MEMBER(mType)
METATABLE_END(Engine::BindingDescriptor)