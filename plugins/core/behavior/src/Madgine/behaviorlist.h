#pragma once

#include "behaviorhandle.h"
#include "parametertuple.h"

namespace Engine {

struct MADGINE_BEHAVIOR_EXPORT BehaviorList : Serialize::SerializableDataUnit {

    void addBehavior(BehaviorHandle handle);

    struct Entry : Serialize::SerializableDataUnit {
        Entry(BehaviorHandle handle);

        BehaviorHandle mHandle;
        ParameterTuple mParameters;
    };

    std::vector<Entry> mEntries;
};

}