#include "../behaviorlib.h"

#include "nameddescriptor.h"

#include "Meta/reflectserialize/valuetypeserialize.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN(Engine::Behavior::NamedDescriptor)
    MEMBER(mName)
    //MEMBER(mType)
METATABLE_END(Engine::Behavior::NamedDescriptor)

SERIALIZETABLE_BEGIN(Engine::Behavior::NamedDescriptor)
    FIELD(mName)
    //FIELD(mType)
SERIALIZETABLE_END(Engine::Behavior::NamedDescriptor)
