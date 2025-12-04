#include "../behaviorlib.h"

#include "behaviorlist.h"

#include "Meta/serialize/helper/typedobjectserialize.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

Engine::Serialize::StreamResult readBehavior(Engine::Serialize::CallerHierarchyFormattedSerializeStream in, Engine::Behavior::BehaviorHandle &handle)
{
    std::string tag;
    STREAM_PROPAGATE_ERROR(Engine::Serialize::beginExtendedTypedRead(in, tag));

    if (!handle.fromString(tag)) {
        return STREAM_INTEGRITY_ERROR(in.mStream) << "Unknown Behavior descriptor: " << tag;
    }
    return {};
}

const char *writeBehavior(Engine::Serialize::CallerHierarchyFormattedSerializeStream out, const Engine::Behavior::BehaviorList::Entry &entry)
{
    static std::string dummy;
    dummy = entry.mHandle.toString();
    return Engine::Serialize::beginExtendedTypedWrite(out, dummy);
}

SERIALIZETABLE_BEGIN(Engine::Behavior::BehaviorList)
    FIELD(mEntries, Engine::Serialize::CustomCreator<readBehavior, writeBehavior>)
SERIALIZETABLE_END(Engine::Behavior::BehaviorList)

SERIALIZETABLE_BEGIN(Engine::Behavior::BehaviorList::Entry)
    FIELD(mParameters)
SERIALIZETABLE_END(Engine::Behavior::BehaviorList::Entry)

METATABLE_BEGIN(Engine::Behavior::BehaviorList)
METATABLE_END(Engine::Behavior::BehaviorList)

namespace Engine {
namespace Behavior {

    void BehaviorList::addBehavior(BehaviorHandle handle)
    {
        mEntries.emplace_back(std::move(handle));
    }

    BehaviorList::Entry::Entry(BehaviorHandle handle)
        : mHandle(std::move(handle))
        , mParameters(mHandle.createParameters())
    {
    }

}
}
