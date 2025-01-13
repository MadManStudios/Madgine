#pragma once

#include "Meta/keyvalue/accessor.h"
#include "Meta/keyvalue/metatable.h"
#include "Meta/serialize/streams/streamresult.h"

#include "Meta/serialize/streams/formattedserializestream.h"

#include "Meta/serialize/operations.h"

#include "Meta/keyvalue/typedscopeptr.h"

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

    virtual std::unique_ptr<ParameterTupleBase> clone() override
    {
        return std::make_unique<ParameterTupleInstance<Ty...>>(mTuple);
    }

    virtual ScopePtr customScopePtr() override
    {
        return { this, &sMetaTable };
    }

    virtual Serialize::StreamResult read(Serialize::FormattedSerializeStream &in) override
    {
        return TupleUnpacker::accumulate(
            mTuple, [&](auto &e, Serialize::StreamResult r) {
                STREAM_PROPAGATE_ERROR(std::move(r));
                return Serialize::readState(in, e, nullptr);
            },
            Serialize::StreamResult {});
    }

    virtual void write(Serialize::FormattedSerializeStream &out) override
    {
        TupleUnpacker::forEach(mTuple, [&](const auto &e) {
            Serialize::writeState(out, e, "Item");
        });
    }

    std::tuple<Ty...> mTuple;

    static const MetaTable *sMetaTablePtr;

    template <typename T>
    static void sGetter(ValueType &retVal, const ScopePtr &scope)
    {
        assert(scope.mType == &sMetaTable);
        to_ValueType(retVal, std::get<T>(static_cast<ParameterTupleInstance *>(scope.mScope)->mTuple));
    }

    template <typename T>
    static void sSetter(const ScopePtr &scope, const ValueType &val)
    {
        assert(scope.mType == &sMetaTable);
        std::get<T>(static_cast<ParameterTupleInstance *>(scope.mScope)->mTuple) = ValueType_as<T>(val);
    }

    static const constexpr std::pair<const char *, Accessor> sMembers[] = {
        { "Item", { &sGetter<Ty>, &sSetter<Ty>, toValueTypeDesc<Ty>() } }...,
        { nullptr, { nullptr, nullptr, ExtendedValueTypeDesc { ExtendedValueTypeEnum::GenericType } } }
    };

    static const constexpr MetaTable sMetaTable {
        &sMetaTablePtr,
        "<ParameterTuple>",
        sMembers
    };
};

template <typename... Ty>
const MetaTable *ParameterTupleInstance<Ty...>::sMetaTablePtr = &ParameterTupleInstance<Ty...>::sMetaTable;

struct MADGINE_BEHAVIOR_EXPORT ParameterTuple {

    ParameterTuple() = default;
    ParameterTuple(const ParameterTuple &other)
        : mTuple(other.mTuple->clone())
    {
    }

    template <typename... Ty>
    ParameterTuple(std::tuple<Ty...> parameters)
        : mTuple(std::make_unique<ParameterTupleInstance<Ty...>>(std::move(parameters)))
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

    friend Serialize::StreamResult tag_invoke(Serialize::apply_map_t, ParameterTuple& tuple, Serialize::FormattedSerializeStream& in, bool success, const CallerHierarchyBasePtr& hierarchy = {}) {
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