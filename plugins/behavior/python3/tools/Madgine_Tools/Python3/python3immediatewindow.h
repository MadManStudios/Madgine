#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

#include "Madgine/debug/debuglistener.h"

#include "Madgine_Tools/texteditor/interactiveprompt.h"

namespace Engine {
namespace Tools {

    struct MADGINE_PYTHON3_TOOLS_EXPORT Python3ImmediateWindow : Tool<Python3ImmediateWindow>, Debug::DebugListener, Interpreter {
        Python3ImmediateWindow(ImRoot &root);

        virtual std::string_view key() const override;

        virtual Threading::Task<bool> init() override;
        virtual Threading::Task<void> finalize() override;

        virtual void renderMenu() override;

        virtual void render() override;

        std::string_view name() override;

    protected:
        bool wantsPause(const Debug::DebugLocation &location, Debug::ContinuationType type) override;
        void onSuspend(Debug::ContextInfo &context, Debug::ContinuationType type) override;

        bool interpret(std::string_view command) override;
        Behavior run(std::string_view command);

    private:
        std::string mCommandBuffer;
        std::ostringstream mCommandLog;

        std::unique_ptr<InteractivePrompt> mPrompt;

        Scripting::Python3::Python3Environment *mEnv = nullptr;
    };

}
}

REGISTER_TYPE(Engine::Tools::Python3ImmediateWindow)