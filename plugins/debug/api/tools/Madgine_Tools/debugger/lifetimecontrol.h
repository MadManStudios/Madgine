#pragma once

#include "Madgine_Tools/toolscollector.h"

#include "Madgine_Tools/toolbase.h"


namespace Engine {
namespace Tools {

    
    namespace ed = ax::NodeEditor;


    struct MADGINE_DEBUGGER_TOOLS_EXPORT LifetimeControl : Tool<LifetimeControl> {

        SERIALIZABLEUNIT(LifetimeControl)

        LifetimeControl(ImRoot &root);
        LifetimeControl(const LifetimeControl &) = delete;

        virtual Threading::Task<bool> init() override;
        virtual Threading::Task<void> finalize() override;

        virtual void update() override;
        virtual void render() override;
        virtual void renderMenu() override;

        std::string_view key() const override;

        void renderTreeView();
        void renderToolbar();

    private:
        std::unique_ptr<ed::EditorContext, void (*)(ed::EditorContext *)> mEditor = { nullptr, nullptr };
    };

}
}

REGISTER_TYPE(Engine::Tools::LifetimeControl)