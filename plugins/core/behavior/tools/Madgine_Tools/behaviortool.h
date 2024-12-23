#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

#include "Madgine/behaviorhandle.h"

namespace Engine {
namespace Tools {

    struct MADGINE_BEHAVIOR_TOOLS_EXPORT BehaviorTool : public Tool<BehaviorTool> {

        BehaviorTool(ImRoot &root);

        Threading::Task<bool> init() override;
        Threading::Task<void> finalize() override;

        void DrawBehaviorList(BehaviorList &list);

        std::string_view key() const override;

    private:
        Inspector *mInspector = nullptr;
    };

    MADGINE_BEHAVIOR_TOOLS_EXPORT BehaviorHandle BehaviorSelector();

}
}

REGISTER_TYPE(Engine::Tools::BehaviorTool)
