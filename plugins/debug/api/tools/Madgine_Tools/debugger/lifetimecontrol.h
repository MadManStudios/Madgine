#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

namespace Engine {
namespace Tools {

    namespace ed = ax::NodeEditor;

    struct MADGINE_DEBUGGER_TOOLS_EXPORT LifetimeControl : Tool<LifetimeControl> {

        SERIALIZABLEUNIT(LifetimeControl)

        LifetimeControl(ImRoot &root);
        LifetimeControl(const LifetimeControl &) = delete;

        Threading::Task<bool> init() override;
        Threading::Task<void> finalize() override;

        void update() override;
        void render() override;
        void renderMenu() override;

        void format();

        std::string_view key() const override;

        void renderTreeView();
        void renderToolbar();

    private:
        std::unique_ptr<ed::EditorContext, void (*)(ed::EditorContext *)> mEditor = { nullptr, nullptr };
    };

}
}