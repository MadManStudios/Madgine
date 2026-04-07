#pragma once
#include "Meta/keyvalue/argumentlist.h"
#include "Meta/keyvalue/boundapifunction.h"
#include "Meta/keyvalue/scopeptr.h"

#include "../toolbase.h"
#include "../toolscollector.h"

// #include "inspectorlayout.h"

namespace Engine {
namespace Tools {

    struct Trace;

    struct MADGINE_TOOLS_EXPORT Inspector : Tool<Inspector> {
        Inspector(ImRoot &root);
        Inspector(const Inspector &) = delete;
        ~Inspector();

        void update() override;
        void render() override;
        void renderMenu() override;

        bool drawRemainingMembers(ScopePtr scope, Trace &trace, std::set<std::string> &drawn);
        bool drawMember(const ScopeIterator &it, Trace &trace);
        std::pair<bool, bool> drawValue(std::string_view id, ValueType &value, bool editable, ExtendedValueTypeDesc possibleType = { ExtendedValueTypeEnum::GenericType });
        std::pair<bool, bool> drawValue(std::string_view id, ValueType &value, bool editable, Trace &trace, ExtendedValueTypeDesc possibleType);
        std::pair<bool, bool> drawValue(std::string_view id, ScopePtr &scope, bool editable, ExtendedValueTypeDesc possibleTypes = { ExtendedValueTypeEnum::GenericType }, ValueTypeDesc *type = nullptr);
        std::pair<bool, bool> drawValue(std::string_view id, ScopePtr &scope, bool editable, Trace &trace, ExtendedValueTypeDesc possibleTypes = { ExtendedValueTypeEnum::GenericType }, ValueTypeDesc *type = nullptr);
        std::pair<bool, bool> drawValue(std::string_view id, OwnedScopePtr &scope, bool editable, ExtendedValueTypeDesc possibleTypes = { ExtendedValueTypeEnum::GenericType }, ValueTypeDesc *type = nullptr);
        std::pair<bool, bool> drawValue(std::string_view id, OwnedScopePtr &scope, bool editable, Trace &trace, ExtendedValueTypeDesc possibleTypes = { ExtendedValueTypeEnum::GenericType }, ValueTypeDesc *type = nullptr);
        std::pair<bool, bool> drawValue(std::string_view id, ObjectPtr &object, bool editable, ExtendedValueTypeDesc possibleTypes = { ExtendedValueTypeEnum::GenericType }, ValueTypeDesc *type = nullptr);
        std::pair<bool, bool> drawValue(std::string_view id, ObjectPtr &object, bool editable, Trace &trace, ExtendedValueTypeDesc possibleTypes = { ExtendedValueTypeEnum::GenericType }, ValueTypeDesc *type = nullptr);
        bool drawValue(std::string_view id, KeyValueVirtualSequenceRange &range, bool editable);
        bool drawValue(std::string_view id, KeyValueVirtualSequenceRange &range, bool editable, Trace &trace);
        bool drawValue(std::string_view id, KeyValueVirtualAssociativeRange &range, bool editable);
        bool drawValue(std::string_view id, KeyValueVirtualAssociativeRange &range, bool editable, Trace &trace);
        void drawValue(std::string_view id, BoundApiFunction &function, bool editable);
        void drawValue(std::string_view id, BoundApiFunction &function, bool editable, Trace &trace);

        bool drawMembers(ScopePtr scope, std::set<std::string> drawn = {});
        bool drawMembers(ScopePtr scope, Trace &trace, std::set<std::string> drawn = {});

        bool drawTypeDecorations(ValueTypeDesc &type, ExtendedValueTypeDesc possibleTypes);

        KeyValueResult followTrace(ValueType &retVal, const Trace &trace);

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
            addPreviewDefinition(table<T>, [preview { forward_capture<F>(preview) }](ScopePtr p) { return preview(scope_cast<T>(p)); });
        }

    private:
        std::map<const MetaTable *, std::function<std::vector<std::pair<std::string_view, ScopePtr>>()>> mPtrSuggestionsByType;
        std::map<const MetaTable *, std::function<bool(ScopePtr)>> mPreviews;

        static std::map<std::string, bool (Inspector::*)(ScopePtr, std::set<std::string> &, tinyxml2::XMLElement *)> sElements;

        std::list<Trace> mViews;

        // FunctionTool
        std::string mCurrentPopupFunctionName;
        BoundApiFunction mCurrentPopupFunction;
        ArgumentList mCurrentPopupArguments;
    };
}
}