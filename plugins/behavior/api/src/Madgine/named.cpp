#include "behaviorlib.h"

#include "named.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"
#include "Meta/keyvalueutil/valuetypeserialize.h"

METATABLE_BEGIN(Engine::NamedDescriptor)
MEMBER(mName)
MEMBER(mType)
METATABLE_END(Engine::NamedDescriptor)

SERIALIZETABLE_BEGIN(Engine::NamedDescriptor)
FIELD(mName)
FIELD(mType)
SERIALIZETABLE_END(Engine::NamedDescriptor)
