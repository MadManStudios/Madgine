#pragma once

#include "behaviordescriptor.h"
#include "parametertuple.h"

namespace Engine {

namespace Behavior {

    MADGINE_BEHAVIOR_EXPORT std::unique_ptr<Reflect::Accessor[]> parameterAccessors(const std::vector<BehaviorDescriptor::Parameter> &parameters);

    struct MADGINE_BEHAVIOR_EXPORT DynamicParameterTuple : ParameterTupleBase {

        DynamicParameterTuple(const Reflect::MetaTable &metaTable, const std::vector<BehaviorDescriptor::Parameter> &parameters);
        DynamicParameterTuple(const DynamicParameterTuple &) = delete;
        DynamicParameterTuple(DynamicParameterTuple &&);
        ~DynamicParameterTuple();

        size_t size() const override;
        std::string_view name(size_t index) const override;
        Reflect::ExtendedType type(size_t index) const override;

        std::unique_ptr<ParameterTupleBase> clone() override;

        Reflect::ScopePtr customScopePtr() override;

        Serialize::StreamResult read(Serialize::FormattedSerializeStream &in, Serialize::ContextPtr context) override;
        void write(Serialize::FormattedSerializeStream &out, Serialize::ContextPtr context) override;
        Serialize::StreamResult applyMap(Serialize::FormattedSerializeStream &in, bool success, Serialize::ContextPtr context) override;

        Reflect::ArgumentList toArgumentList() override;
        Reflect::Result fromArgumentList(const Reflect::ArgumentList &args) override;

        const Reflect::MetaTable &mMetaTable;
        std::vector<Type::InlineStorage> mValues;
    };

}

}