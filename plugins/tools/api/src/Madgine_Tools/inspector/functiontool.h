#pragma once

#include "../toolbase.h"
#include "../toolscollector.h"
#include "Meta/keyvalue/boundapifunction.h"
#include "Meta/keyvalue/argumentlist.h"

namespace Engine {
namespace Tools {

    struct MADGINE_TOOLS_EXPORT FunctionTool : Tool<FunctionTool> {
        FunctionTool(ImRoot &root);
        ~FunctionTool();

        std::string_view key() const override;

        virtual Threading::Task<bool> init() override;

        virtual void render() override;

        void setCurrentFunction(std::string_view name, const BoundApiFunction &method);

        bool renderFunction(BoundApiFunction &function, std::string_view functionName, ArgumentList &args);
        bool renderFunctionSelect(BoundApiFunction &function, std::string &functionName, ArgumentList &args);

    protected:
        bool renderFunctionDetails(BoundApiFunction &function, ArgumentList &args);

    private:
        std::string mCurrentFunctionName;
        BoundApiFunction mCurrentFunction;
        ArgumentList mCurrentArguments;

        std::vector<std::pair<std::string, BoundApiFunction>> mMethodCache;

        Inspector *mInspector;
    };

}
}

REGISTER_TYPE(Engine::Tools::FunctionTool)