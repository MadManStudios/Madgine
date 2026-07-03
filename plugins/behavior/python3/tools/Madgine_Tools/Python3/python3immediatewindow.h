#pragma once


#include "Madgine_Tools/texteditor/interactiveprompt.h"
#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

namespace Engine {
namespace Tools {

    struct MADGINE_PYTHON3_TOOLS_EXPORT Python3ImmediateWindow : Tool<Python3ImmediateWindow>, Interpreter {
        Python3ImmediateWindow(ImRoot &root);

        virtual std::string_view key() const override;

        virtual Threading::Task<bool> init() override;
        virtual Threading::Task<void> finalize() override;

        virtual void renderMenu() override;

        virtual void render() override;

        std::string_view name() override;

    protected:

        bool interpret(EMSCRIPTEN_WORKAROUND(std::string_view) command) override;        

    private:
        std::string mCommandBuffer;
        std::ostringstream mCommandLog;

        std::unique_ptr<InteractivePrompt> mPrompt;

        Behavior::Python3::Python3Environment *mEnv = nullptr;
    };

}
}