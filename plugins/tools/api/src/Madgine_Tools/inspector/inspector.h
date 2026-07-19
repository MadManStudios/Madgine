#pragma once
#include "Meta/reflect/argumentlist.h"
#include "Meta/reflect/boundapifunction.h"
#include "Meta/reflect/scopeptr.h"

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

        bool drawRemainingMembers(const Traced<const Reflect::ScopePtr &> &scope, std::set<std::string> &drawn);
        bool drawMember(const Traced<Reflect::ScopeIterator> &it);
        bool drawValue(std::string_view id, const Traced<Reflect::Value &> &value, bool editable, Reflect::ExtendedType possibleType = { Reflect::ExtendedTypeEnum::GenericType });
        bool drawValue(std::string_view id, const Traced<Reflect::ScopePtr &> &scope, bool isOwned, bool editable, Reflect::ExtendedType possibleTypes = { Reflect::ExtendedTypeEnum::GenericType }, Reflect::Type *type = nullptr);
        bool drawValue(std::string_view id, const Traced<Reflect::OwnedValue &> &value, bool editable, Reflect::ExtendedType possibleTypes = { Reflect::ExtendedTypeEnum::GenericType }, Reflect::Type *type = nullptr);
        bool drawValue(std::string_view id, const Traced<Reflect::ObjectPtr &> &object, bool editable, Reflect::ExtendedType possibleTypes = { Reflect::ExtendedTypeEnum::GenericType }, Reflect::Type *type = nullptr);
        bool drawValue(std::string_view id, const Traced<Reflect::SequenceRange &> &range, bool editable);
        bool drawValue(std::string_view id, const Traced<Reflect::AssociativeRange &> &range, bool editable);
        void drawValue(std::string_view id, const Traced<Reflect::BoundApiFunction &> &function, bool editable);
        bool drawValue(std::string_view id, const Traced<Reflect::ScopeBinding &> &binding, bool editable, Reflect::ExtendedType possibleType = { Reflect::ExtendedTypeEnum::GenericType }, Reflect::Type *type = nullptr);

        bool drawMembers(const Traced<const Reflect::ScopePtr &> &scope, std::set<std::string> drawn = {});

        bool drawTypeDecorations(Reflect::Type &type, Reflect::ExtendedType possibleTypes);

        std::string_view key() const override;

        void addTypeHandler(const Reflect::MetaTable *type, std::function<bool(const Traced<Reflect::ScopePtr &> & scope, bool editable)> getter);
        template <typename T>
        void addTypeHandler(std::function<bool(const Traced<Reflect::ScopePtr &> &scope, bool editable)> getter)
        {
            addTypeHandler(table<T>, std::move(getter));
        }
        bool hasTypeHandler(const Reflect::MetaTable *type) const;

        void addPreviewDefinition(const Reflect::MetaTable *type, std::function<bool(const Traced<const Reflect::ScopePtr &> &)> preview);
        template <typename T, typename F>
        void addPreviewDefinition(F &&preview)
        {
            addPreviewDefinition(table<T>, [preview { forward_capture<F>(preview) }](const Traced<const Reflect::ScopePtr &> &p) { return preview(p.trace(&Reflect::scope_cast<T>)); });
        }

        void pushFlags(Reflect::AccessorFlags flags);
        void popFlags();
        Reflect::AccessorFlags flags() const;

    private:
        std::map<const Reflect::MetaTable *, std::function<bool(const Traced<Reflect::ScopePtr &> &scope, bool editable)>> mTypeHandlers;
        std::map<const Reflect::MetaTable *, std::function<bool(const Traced<const Reflect::ScopePtr &> &)>> mPreviews;

        static std::map<std::string, bool (Inspector::*)(Reflect::ScopePtr, std::set<std::string> &, tinyxml2::XMLElement *)> sElements;

        std::list<Trace> mViews;

        std::vector<Reflect::AccessorFlags> mAccessorFlagsStack;

        // FunctionTool
        std::string mCurrentPopupFunctionName;
        Reflect::BoundApiFunction mCurrentPopupFunction;
        Reflect::ArgumentList mCurrentPopupArguments;
    };
}
}