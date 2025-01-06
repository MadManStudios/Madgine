#pragma once

#include "behaviorhandle.h"
#include "parametertuple.h"

namespace Engine {

struct MADGINE_BEHAVIOR_EXPORT BehaviorList : Serialize::SerializableDataUnit {

    void addBehavior(BehaviorHandle handle);

    template <typename Lifetime>
    void instantiate(Lifetime& lifetime) {
        for (const Entry& entry : mEntries) {
            lifetime.attach(entry.mHandle.create(entry.mParameters));
        }
    }

    struct Entry : Serialize::SerializableDataUnit {
        Entry(BehaviorHandle handle);

        BehaviorHandle mHandle;
        ParameterTuple mParameters;
    };

    std::vector<Entry> mEntries;
};

}