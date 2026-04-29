#pragma once
#include "Meta/keyvalue/argumentlist.h"
#include "Meta/keyvalue/boundapifunction.h"
#include "Meta/keyvalue/scopeptr.h"

#include "../toolbase.h"
#include "../toolscollector.h"
#include "../util/trace.h"

// #include "inspectorlayout.h"

namespace Engine {
namespace Tools {

    struct MADGINE_TOOLS_EXPORT Inspector : Tool<Inspector> {
        Inspector(ImRoot &root);
        Inspector(const Inspector &) = delete;
        ~Inspector();

        void update() override;
        void render() override;
        void renderMenu() override;

        bool drawRemainingMembers(const Traced<const ScopePtr &> &scope, std::set<std::string> &drawn);
        bool drawMember(const Traced<ScopeIterator> &it);
        bool drawValue(std::string_view id, const Traced<ValueType &> &value, bool editable, ExtendedValueTypeDesc possibleType = { ExtendedValueTypeEnum::GenericType });
        bool drawValue(std::string_view id, const Traced<ScopePtr &> &scope, bool isOwned, bool editable, ExtendedValueTypeDesc possibleTypes = { ExtendedValueTypeEnum::GenericType }, ValueTypeDesc *type = nullptr);
        bool drawValue(std::string_view id, const Traced<OwnedScopePtr &> &scope, bool editable, ExtendedValueTypeDesc possibleTypes = { ExtendedValueTypeEnum::GenericType }, ValueTypeDesc *type = nullptr);
        bool drawValue(std::string_view id, const Traced<ObjectPtr &> &object, bool editable, ExtendedValueTypeDesc possibleTypes = { ExtendedValueTypeEnum::GenericType }, ValueTypeDesc *type = nullptr);
        bool drawValue(std::string_view id, const Traced<KeyValueVirtualSequenceRange &> &range, bool editable);
        bool drawValue(std::string_view id, const Traced<KeyValueVirtualAssociativeRange &> &range, bool editable);
        void drawValue(std::string_view id, const Traced<BoundApiFunction &> &function, bool editable);

        bool drawMembers(const Traced<const ScopePtr &> &scope, std::set<std::string> drawn = {});

        bool drawTypeDecorations(ValueTypeDesc &type, ExtendedValueTypeDesc possibleTypes);

        std::string_view key() const override;

        void addPtrSuggestion(const MetaTable *type, std::function<std::vector<std::pair<std::string_view, ScopePtr>>()> getter);
        template <typename T>
        void addPtrSuggestion(std::function<std::vector<std::pair<std::string_view, ScopePtr>>()> getter)
        {
            addPtrSuggestion(table<T>, std::move(getter));
        }
        bool hasPtrSuggestion(const MetaTable *type) const;

        void addPreviewDefinition(const MetaTable *type, std::function<bool(const Traced<const ScopePtr &> &)> preview);
        template <typename T, typename F>
        void addPreviewDefinition(F &&preview)
        {
            addPreviewDefinition(table<T>, [preview { forward_capture<F>(preview) }](const Traced<const ScopePtr &> &p) { return preview(p.trace(&scope_cast<T>)); });
        }

        void pushFlags(AccessorFlags flags);
        void popFlags();
        AccessorFlags flags() const;

    private:
        std::map<const MetaTable *, std::function<std::vector<std::pair<std::string_view, ScopePtr>>()>> mPtrSuggestionsByType;
        std::map<const MetaTable *, std::function<bool(const Traced<const ScopePtr &> &)>> mPreviews;

        static std::map<std::string, bool (Inspector::*)(ScopePtr, std::set<std::string> &, tinyxml2::XMLElement *)> sElements;

        std::list<Trace> mViews;

        std::vector<AccessorFlags> mAccessorFlagsStack;

        // FunctionTool
        std::string mCurrentPopupFunctionName;
        BoundApiFunction mCurrentPopupFunction;
        ArgumentList mCurrentPopupArguments;
    };
}
}