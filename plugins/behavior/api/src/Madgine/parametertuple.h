#pragma once

#include "Meta/keyvalue/accessor.h"
#include "Meta/keyvalue/metatable.h"
#include "Meta/serialize/streams/streamresult.h"

#include "Meta/serialize/operations.h"

#include "Meta/keyvalue/scopeptr.h"

namespace Engine {

struct ParameterTupleBase {
    virtual std::unique_ptr<ParameterTupleBase> clone() = 0;
    virtual ScopePtr customScopePtr() = 0;

    virtual Serialize::StreamResult read(Serialize::FormattedSerializeStream &in) = 0;
    virtual void write(Serialize::FormattedSerializeStream &out) = 0;

    virtual ~ParameterTupleBase() = default;
};

template <typename... Ty>
struct ParameterTupleInstance : ParameterTupleBase {
    ParameterTupleInstance(std::tuple<Ty...> tuple)
        : mTuple(std::move(tuple))
    {
    }

    std::tuple<Ty...> mTuple;
};

template <typename Names, typename... Ty>
struct TypedParameterTupleInstance : ParameterTupleInstance<Ty...> {

    using ParameterTupleInstance<Ty...>::ParameterTupleInstance;

    virtual std::unique_ptr<ParameterTupleBase> clone() override
    {
        return std::make_unique<TypedParameterTupleInstance<Names, Ty...>>(this->mTuple);
    }

    virtual ScopePtr customScopePtr() override
    {
        return { this, &sMetaTable };
    }

    virtual Serialize::StreamResult read(Serialize::FormattedSerializeStream &in) override
    {
        return TupleUnpacker::accumulate(
            this->mTuple, [&](auto &e, Serialize::StreamResult r) {
                STREAM_PROPAGATE_ERROR(std::move(r));
                return Serialize::read(in, e, nullptr);
            },
            Serialize::StreamResult {});
    }

    virtual void write(Serialize::FormattedSerializeStream &out) override
    {
        [this, &out] <size_t... Is>(auto_pack<Is...>) {
            (Serialize::write(out, std::get<Is>(this->mTuple), Names::template get<Is>.c_str()), ...);
        }(index_pack_for<Ty...> {});
    }

    static const MetaTable *sMetaTablePtr;

    template <size_t I>
    static void sGetter(ValueType &retVal, const ScopePtr &scope)
    {
        assert(scope.mType == &sMetaTable);
        to_ValueType(retVal, std::get<I>(static_cast<TypedParameterTupleInstance *>(scope.mScope)->mTuple));
    }

    template <size_t I, typename T>
    static void sSetter(const ScopePtr &scope, const ValueType &val)
    {
        assert(scope.mType == &sMetaTable);
        std::get<I>(static_cast<TypedParameterTupleInstance *>(scope.mScope)->mTuple) = ValueType_as<T>(val);
    }

    static const constexpr auto sMembers = []<size_t... Is>(auto_pack<Is...>) constexpr -> std::array<std::pair<const char *, Accessor>, sizeof...(Ty) + 1>
    {
        return { { { Names::template get<Is>.c_str(), { &sGetter<Is>, &sSetter<Is, Ty>, toValueTypeDesc<Ty>() } }...,
            { nullptr, { nullptr, nullptr, ExtendedValueTypeDesc { ExtendedValueTypeEnum::GenericType } } } } };
    }
    (index_pack_for<Ty...> {});

    static const constexpr MetaTable sMetaTable {
        &sMetaTablePtr,
        "<ParameterTuple>",
        sMembers.data()
    };
};

template <typename Names, typename... Ty>
const MetaTable *TypedParameterTupleInstance<Names, Ty...>::sMetaTablePtr = &TypedParameterTupleInstance<Names, Ty...>::sMetaTable;

struct MADGINE_BEHAVIOR_EXPORT ParameterTuple {

    ParameterTuple() = default;
    ParameterTuple(const ParameterTuple &other)
        : mTuple(other.mTuple->clone())
    {
    }

    template <typename... Ty, auto... Names>
    ParameterTuple(std::tuple<Ty...> parameters, auto_pack<Names...>)
        : mTuple(std::make_unique<TypedParameterTupleInstance<auto_pack<Names...>, Ty...>>(std::move(parameters)))
    {
    }

    ParameterTuple &operator=(const ParameterTuple &other)
    {
        mTuple = other.mTuple->clone();
        return *this;
    }

    ScopePtr customScopePtr();

    template <typename... Ty>
    bool get(std::tuple<Ty...> &out) const
    {
        ParameterTupleInstance<Ty...> *instance = dynamic_cast<ParameterTupleInstance<Ty...> *>(mTuple.get());
        if (instance) {
            out = instance->mTuple;
        }
        return instance;
    }

    void reset()
    {
        mTuple.reset();
    }

    explicit operator bool() const
    {
        return static_cast<bool>(mTuple);
    }

private:
    friend struct Serialize::Operations<ParameterTuple>;

    friend Serialize::StreamResult tag_invoke(Serialize::apply_map_t, ParameterTuple &tuple, Serialize::FormattedSerializeStream &in, bool success, const CallerHierarchyBasePtr &hierarchy = {})
    {
        return {};
    }

    std::unique_ptr<ParameterTupleBase> mTuple;
};

namespace Serialize {
    template <>
    struct MADGINE_BEHAVIOR_EXPORT Operations<ParameterTuple> {
        static StreamResult read(Serialize::FormattedSerializeStream &in, ParameterTuple &tuple, const char *name);
        static void write(Serialize::FormattedSerializeStream &out, const ParameterTuple &tuple, const char *name);

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor);
    };
}

}