#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

#include "Madgine/behaviorhandle.h"

namespace Engine {
namespace Tools {

    struct BehaviorTool : public Tool<BehaviorTool> {

        BehaviorTool(ImRoot &root);

        Threading::Task<bool> init() override;
        Threading::Task<void> finalize() override;

        std::string_view key() const override;
    };

}
}

REGISTER_TYPE(Engine::Tools::BehaviorTool)

namespace ImGui {
MADGINE_BEHAVIOR_TOOLS_EXPORT Engine::BehaviorHandle BehaviorSelector();
}
