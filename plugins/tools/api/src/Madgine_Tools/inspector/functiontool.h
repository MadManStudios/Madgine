#pragma once

#include "Meta/reflect/argumentlist.h"
#include "Meta/reflect/boundapifunction.h"

#include "../toolbase.h"
#include "../toolscollector.h"
#include "../util/undostack.h"

namespace Engine {
namespace Tools {

    struct MADGINE_TOOLS_EXPORT FunctionTool : Tool<FunctionTool> {
        FunctionTool(ImRoot &root);
        ~FunctionTool();

        std::string_view key() const override;

        virtual Threading::Task<bool> init() override;

        virtual void render() override;

        void setCurrentFunction(std::string_view name, const Reflect::BoundApiFunction &method);

        bool renderFunction(const Traced<Reflect::BoundApiFunction &> &function, std::string_view functionName, Reflect::ArgumentList &args);
        bool renderFunctionSelect(const Traced<Reflect::BoundApiFunction &> &function, std::string &functionName, Reflect::ArgumentList &args);

    protected:
        bool renderFunctionDetails(const Traced<Reflect::BoundApiFunction &> &function, Reflect::ArgumentList &args);

    private:
        std::string mCurrentFunctionName;
        Reflect::BoundApiFunction mCurrentFunction;
        Reflect::ArgumentList mCurrentArguments;

        std::vector<std::pair<std::string, Reflect::BoundApiFunction>> mMethodCache;

        Inspector *mInspector;

        UndoStack mHistory;
    };

}
}