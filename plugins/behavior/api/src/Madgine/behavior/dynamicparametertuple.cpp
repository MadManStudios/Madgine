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
                [](const Reflect::Accessor *self, Reflect::Value &out, const Reflect::Value &scope) -> Reflect::Result {
                    size_t index = self - (*scope.type().mSecondary.mMetaTable)->mMembers;
                    return call([&, index](DynamicParameterTuple &tuple) -> Reflect::Result { tuple.mValues[index].toValue(out); return {}; }, scope);
                },
                [](const Reflect::Accessor *self, const Reflect::Value &scope, const Reflect::Value &val) -> Reflect::Result {
                    size_t index = self - (*scope.type().mSecondary.mMetaTable)->mMembers;
                    return call([&](DynamicParameterTuple &tuple) { return tuple.mValues[index].fromValue(val); }, scope);
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

    Serialize::StreamResult DynamicParameterTuple::read(Serialize::CallerHierarchyFormattedSerializeStream in)
    {
        for (size_t i = 0; i < mValues.size(); ++i) {
            STREAM_PROPAGATE_ERROR(Serialize::read(in, mValues[i], mMetaTable.mMembers[i].mName));
        }
        return {};
    }

    void DynamicParameterTuple::write(Serialize::CallerHierarchyFormattedSerializeStream out)
    {
        for (size_t i = 0; i < mValues.size(); ++i) {
            Serialize::write(out, mValues[i], mMetaTable.mMembers[i].mName);
        }
    }

    Serialize::StreamResult DynamicParameterTuple::applyMap(Serialize::CallerHierarchyFormattedSerializeStream in, bool success)
    {
        for (size_t i = 0; i < mValues.size(); ++i) {
            STREAM_PROPAGATE_ERROR(Serialize::apply_map(mValues[i], in, success));
        }
        return {};
    }

    Reflect::ArgumentList DynamicParameterTuple::toArgumentList()
    {
        Reflect::ArgumentList result { std::true_type {}, mValues.size() };
        for (size_t i = 0; i < mValues.size(); ++i) {
            mValues[i].toValue(result[i]);
        }
        return result;
    }

}
}