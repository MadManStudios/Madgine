#include "../behaviorlib.h"

#include "dynamicparametertuple.h"

#include "Meta/reflect/util.h"
#include "Meta/reflect/value.h"
#include "Meta/type/storageops.h"

#include "Meta/reflect/metatable_impl.h"

#include "behaviordescriptor.h"

METATABLE_BEGIN(Engine::Behavior::DynamicParameterTuple)
METATABLE_END(Engine::Behavior::DynamicParameterTuple);

namespace Engine {
namespace Behavior {

    std::unique_ptr<Reflect::Accessor[]> parameterAccessors(const std::vector<BehaviorDescriptor::Parameter> &parameters)
    {
        std::unique_ptr<Reflect::Accessor[]> accessors = std::make_unique<Reflect::Accessor[]>(parameters.size() + 1);

        for (size_t i = 0; i < parameters.size(); ++i) {
            accessors[i] = Reflect::Accessor {
                parameters[i].mName.data(),
                nullptr,
                [](const Reflect::Accessor *self, Reflect::Value &out, const Reflect::Value &scope, Reflect::ContextPtr context) -> Reflect::Result {
                    size_t index = self - (*scope.type().mSecondary.mMetaTable)->mMembers;
                    return call([&, index](DynamicParameterTuple &tuple) -> Reflect::Result { tuple.mValues[index].toValue(out); return {}; }, scope, context);
                },
                [](const Reflect::Accessor *self, const Reflect::Value &scope, const Reflect::Value &val, Reflect::ContextPtr context) -> Reflect::Result {
                    size_t index = self - (*scope.type().mSecondary.mMetaTable)->mMembers;
                    return call([&](DynamicParameterTuple &tuple) { return tuple.mValues[index].fromValue(val); }, scope, context);
                },
                (*parameters[i].mType)->mType
            };
        }

        return accessors;
    }

    DynamicParameterTuple::DynamicParameterTuple(const Reflect::MetaTable &metaTable, const std::vector<BehaviorDescriptor::Parameter> &parameters)
        : mMetaTable(metaTable)
    {
        mValues.reserve(parameters.size());

        for (const BehaviorDescriptor::Parameter &param : parameters) {
            mValues.emplace_back(**param.mType, Reflect::ArgumentList {});
        }
    }

    DynamicParameterTuple::DynamicParameterTuple(DynamicParameterTuple &&) = default;

    DynamicParameterTuple::~DynamicParameterTuple()
    {
    }

    size_t DynamicParameterTuple::size() const
    {
        return mValues.size();
    }

    std::string_view DynamicParameterTuple::name(size_t index) const
    {
        return mMetaTable.mMembers[index].mName;
    }

    Reflect::ExtendedType DynamicParameterTuple::type(size_t index) const
    {
        return mMetaTable.mMembers[index].mType;
    }

    std::unique_ptr<ParameterTupleBase> DynamicParameterTuple::clone()
    {
        throw "TODO";
        //return std::make_unique<DynamicParameterTuple>(*this);
    }

    Reflect::ScopePtr DynamicParameterTuple::customScopePtr()
    {
        return { this, &mMetaTable };
    }

    Serialize::StreamResult DynamicParameterTuple::read(Serialize::FormattedSerializeStream &in, Serialize::ContextPtr context)
    {
        for (size_t i = 0; i < mValues.size(); ++i) {
            STREAM_PROPAGATE_ERROR(Serialize::read(in, mValues[i], mMetaTable.mMembers[i].mName, context));
        }
        return {};
    }

    void DynamicParameterTuple::write(Serialize::FormattedSerializeStream &out, Serialize::ContextPtr context)
    {
        for (size_t i = 0; i < mValues.size(); ++i) {
            Serialize::write(out, mValues[i], mMetaTable.mMembers[i].mName, context);
        }
    }

    Serialize::StreamResult DynamicParameterTuple::applyMap(Serialize::FormattedSerializeStream &in, bool success, Serialize::ContextPtr context)
    {
        for (size_t i = 0; i < mValues.size(); ++i) {
            STREAM_PROPAGATE_ERROR(Serialize::apply_map(mValues[i], in, success, context));
        }
        return {};
    }

    void DynamicParameterTuple::get(Reflect::Value &retVal, size_t index)
    {
        mValues[index].toValue(retVal);
    }

    Reflect::ArgumentList DynamicParameterTuple::toArgumentList()
    {
        Reflect::ArgumentList result { std::true_type {}, mValues.size() };
        for (size_t i = 0; i < mValues.size(); ++i) {
            mValues[i].toValue(result[i]);
        }
        return result;
    }

    Reflect::Result DynamicParameterTuple::fromArgumentList(const Reflect::ArgumentList &args)
    {
        for (size_t i = 0; i < mValues.size(); ++i) {            
            REFLECT_PROPAGATE_ERROR(mValues[i].fromValue(args.at(i)));
        }
        return {};
    }

}
}