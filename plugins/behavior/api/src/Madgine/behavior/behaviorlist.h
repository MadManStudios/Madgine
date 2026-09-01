#pragma once

#include "Platform/log/logsenders.h"

#include "behaviorsender.h"

namespace Engine {
namespace Behavior {

    struct MADGINE_BEHAVIOR_EXPORT BehaviorList {

        void addBehavior(BehaviorSender behavior);

        template <typename Lifetime>
        void instantiate(Lifetime &lifetime)
        {
            for (const BehaviorSender &sender: mEntries) {
                lifetime.attach(sender.create() | Platform::Log::log_result());
            }
        }

        std::vector<BehaviorSender> mEntries;
    };

}
}