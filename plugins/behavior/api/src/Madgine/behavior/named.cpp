#include "../behaviorlib.h"

#include "named.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"
#include "Meta/keyvalueutil/valuetypeserialize.h"

METATABLE_BEGIN(Engine::Behavior::NamedDescriptor)
MEMBER(mName)
MEMBER(mType)
METATABLE_END(Engine::Behavior::NamedDescriptor)

SERIALIZETABLE_BEGIN(Engine::Behavior::NamedDescriptor)
FIELD(mName)
FIELD(mType)
SERIALIZETABLE_END(Engine::Behavior::NamedDescriptor)
