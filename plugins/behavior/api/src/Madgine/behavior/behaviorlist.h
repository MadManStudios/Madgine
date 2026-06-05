#pragma once

#include "Platform/log/logsenders.h"

#include "behavior.h"
#include "behaviorhandle.h"
#include "parametertuple.h"

namespace Engine {
namespace Behavior {

    struct MADGINE_BEHAVIOR_EXPORT BehaviorList {

        void addBehavior(BehaviorHandle handle);

        template <typename Lifetime>
        void instantiate(Lifetime &lifetime)
        {
            for (const Entry &entry : mEntries) {
                lifetime.attach(entry.mHandle.create(entry.mParameters) | Platform::Log::log_result());
            }
        }

        struct Entry {
            Entry(BehaviorHandle handle);

            BehaviorHandle mHandle;
            ParameterTuple mParameters;
        };

        std::vector<Entry> mEntries;
    };

}
}