#pragma once

#include "Madgine/debug/debuglistener.h"

#include "Madgine_Tools/texteditor/interactiveprompt.h"
#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

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
        bool wantsPause(Debug::ContextInfo &context, TypedPtr location, Debug::ContinuationType type, IndexType<size_t> line) override;
        void onSuspend(Debug::ContextInfo &context, TypedPtr location, Debug::ContinuationType type) override;

        bool interpret(std::string_view command) override;
        Behavior::Behavior run(std::string_view command);

    private:
        std::string mCommandBuffer;
        std::ostringstream mCommandLog;

        std::unique_ptr<InteractivePrompt> mPrompt;

        Behavior::Python3::Python3Environment *mEnv = nullptr;
    };

}
}