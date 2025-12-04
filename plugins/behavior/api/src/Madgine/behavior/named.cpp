#include "../behaviorlib.h"

#include "named.h"

#include "Meta/keyvalueutil/valuetypeserialize.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN(Engine::Behavior::NamedDescriptor)
    MEMBER(mName)
    MEMBER(mType)
METATABLE_END(Engine::Behavior::NamedDescriptor)

SERIALIZETABLE_BEGIN(Engine::Behavior::NamedDescriptor)
    FIELD(mName)
    FIELD(mType)
SERIALIZETABLE_END(Engine::Behavior::NamedDescriptor)
