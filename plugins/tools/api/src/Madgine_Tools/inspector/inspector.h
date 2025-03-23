#pragma once
#include "../toolbase.h"
#include "../toolscollector.h"

#include "Meta/keyvalue/boundapifunction.h"
#include "Meta/keyvalue/scopeptr.h"

#include "Meta/keyvalue/argumentlist.h"

//#include "inspectorlayout.h"

namespace Engine {
namespace Tools {

    struct MADGINE_TOOLS_EXPORT Inspector : Tool<Inspector> {
        Inspector(ImRoot &root);
        Inspector(const Inspector &) = delete;
        ~Inspector();

        virtual void render() override;

        bool drawRemainingMembers(ScopePtr scope, std::set<std::string> &drawn);
        bool drawMember(ScopePtr parent, const ScopeIterator &it);
        std::pair<bool, bool> drawValue(std::string_view id, ValueType &value, bool editable, bool generic = false);
        std::pair<bool, bool> drawValue(std::string_view id, ScopePtr &scope, bool editable, ValueType *generic = nullptr);
        std::pair<bool, bool> drawValue(std::string_view id, OwnedScopePtr &scope, bool editable, ValueType *generic = nullptr);
        std::pair<bool, bool> drawValue(std::string_view id, ObjectPtr &object, bool editable, ValueType *generic = nullptr);
        bool drawValue(std::string_view id, KeyValueVirtualSequenceRange &range, bool editable);
        bool drawValue(std::string_view id, KeyValueVirtualAssociativeRange &range, bool editable);
        void drawValue(std::string_view id, BoundApiFunction &function, bool editable);

        bool drawMembers(ScopePtr scope, std::set<std::string> drawn = {});

        std::string_view key() const override;

        void addPtrSuggestion(const MetaTable *type, std::function<std::vector<std::pair<std::string_view, ScopePtr>>()> getter);
        template <typename T>
        void addPtrSuggestion(std::function<std::vector<std::pair<std::string_view, ScopePtr>>()> getter)
        {
            addPtrSuggestion(table<T>, std::move(getter));
        }
        bool hasPtrSuggestion(const MetaTable *type) const;

        void addPreviewDefinition(const MetaTable *type, std::function<bool(ScopePtr)> preview);
        template <typename T, typename F>
        void addPreviewDefinition(F &&preview)
        {
            addPreviewDefinition(table<T>, [preview { forward_capture(std::forward<F>(preview)) }](ScopePtr p) { return preview(scope_cast<T>(p)); });
        }

    private:
        std::map<const MetaTable *, std::function<std::vector<std::pair<std::string_view, ScopePtr>>()>> mPtrSuggestionsByType;
        std::map<const MetaTable *, std::function<bool(ScopePtr)>> mPreviews;

        static std::map<std::string, bool (Inspector::*)(ScopePtr, std::set<std::string> &, tinyxml2::XMLElement *)> sElements;

        //FunctionTool
        std::string mCurrentPopupFunctionName;
        BoundApiFunction mCurrentPopupFunction;
        ArgumentList mCurrentPopupArguments;
    };
}
}

REGISTER_TYPE(Engine::Tools::Inspector)