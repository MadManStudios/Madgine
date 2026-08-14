#pragma once

#include "Meta/reflect/accessor.h"
#include "Meta/reflect/metatable.h"
#include "Meta/reflect/scopeptr.h"
#include "Meta/reflect/util.h"
#include "Meta/serialize/operations.h"

#include "parametertuple.h"

namespace Engine {
namespace Behavior {

    template <typename... Ty>
    struct ParameterTupleInstance : ParameterTupleBase {
        ParameterTupleInstance(std::tuple<Ty...> tuple)
            : mTuple(std::move(tuple))
        {
        }

        size_t size() const override
        {
            return sizeof...(Ty);
        }

        Reflect::ExtendedType type(size_t index) const override
        {
            if constexpr (sizeof...(Ty) == 0) {
                throw 0;
            } else {
                return TupleUnpacker::select(mTuple, [](auto &&a) { return Reflect::toType<std::decay_t<decltype(a)>>(); }, index);
            }
        }

        std::tuple<Ty...> mTuple;
    };

    template <typename Names, typename... Ty>
    struct TypedParameterTupleInstance : ParameterTupleInstance<Ty...> {

        using ParameterTupleInstance<Ty...>::ParameterTupleInstance;

        static const constexpr auto sNames = []<size_t... Is>(auto_pack<Is...>) constexpr -> std::array<std::string_view, sizeof...(Ty)>
        {
            return {
                Names::template get<Is>...
            };
        }
        (index_pack_for<Ty...> {});

        std::string_view name(size_t index) const override
        {
            return sNames[index];
        }

        std::unique_ptr<ParameterTupleBase> clone() override
        {
            return std::make_unique<TypedParameterTupleInstance<Names, Ty...>>(this->mTuple);
        }

        Reflect::ScopePtr customScopePtr() override
        {
            return { this, sMetaTablePtr };
        }

        Reflect::ArgumentList toArgumentList() override
        {
            return [this]<size_t... Is>(auto_pack<Is...>) {
                return Reflect::ArgumentList { std::get<Is>(this->mTuple)... };
            }(index_pack_for<Ty...> {});
        }

        Serialize::StreamResult read(Serialize::FormattedSerializeStream &in, Serialize::ContextPtr context) override
        {
            std::tuple<dependent_t<Serialize::StreamResult, Ty>...> results;
            [&]<size_t... Is>(auto_pack<Is...>) {
                ([&]() { std::get<Is>(results) = Serialize::read(in, std::get<Is>(this->mTuple), Names::template get<Is>.c_str(), context); }(), ...);
            }(index_pack_for<Ty...> {});

            return TupleUnpacker::accumulate(std::move(results), [](Serialize::StreamResult first, Serialize::StreamResult second) {
                    STREAM_PROPAGATE_ERROR(std::move(first));
                    return std::move(second); }, Serialize::StreamResult {});
        }

        void write(Serialize::FormattedSerializeStream &out, Serialize::ContextPtr context) override
        {
            [&]<size_t... Is>(auto_pack<Is...>) {
                (Serialize::write(out, std::get<Is>(this->mTuple), Names::template get<Is>.c_str(), context), ...);
            }(index_pack_for<Ty...> {});
        }

        Serialize::StreamResult applyMap(Serialize::FormattedSerializeStream &in, bool success, Serialize::ContextPtr context) override
        {
            std::tuple<dependent_t<Serialize::StreamResult, Ty>...> results;
            [&]<size_t... Is>(auto_pack<Is...>) {
                ([&]() { std::get<Is>(results) = Serialize::apply_map(std::get<Is>(this->mTuple), in, success, context); }(), ...);
            }(index_pack_for<Ty...> {});

            return TupleUnpacker::accumulate(std::move(results), [](Serialize::StreamResult first, Serialize::StreamResult second) {
                    STREAM_PROPAGATE_ERROR(std::move(first));
                    return std::move(second); }, Serialize::StreamResult {});
        }

        static const Reflect::MetaTable *sMetaTablePtr;

        template <size_t I>
        static Reflect::Result sGetter(const Reflect::Accessor *, Reflect::Value &retVal, const Reflect::Value &scope, Reflect::ContextPtr context)
        {
            return invoke_member(retVal, [](TypedParameterTupleInstance &instance) -> decltype(auto) { return std::get<I>(instance.mTuple); }, context, scope);
        }

        template <size_t I, typename T>
        static Reflect::Result sSetter(const Reflect::Accessor *, const Reflect::Value &scope, const Reflect::Value &val, Reflect::ContextPtr context)
        {
            return invoke_member([](TypedParameterTupleInstance &instance, T value) { std::get<I>(instance.mTuple) = std::move(value); }, context, scope, val);
        }

        static const constexpr auto sMembers = []<size_t... Is>(auto_pack<Is...>) constexpr -> std::array<Reflect::Accessor, sizeof...(Ty) + 1>
        {
            return { { { Names::template get<Is>.c_str(), nullptr, &sGetter<Is>, &sSetter<Is, Ty>, Reflect::toType<Ty>(), Concepts::InstanceOfA1<Ty, Named> ? Reflect::AccessorFlags_Named : Reflect::AccessorFlags_Default }...,
                {} } };
        }
        (index_pack_for<Ty...> {});
    };

}
}

template <typename Names, typename... Ty>
constexpr const Engine::Reflect::MetaTable table_instance<Engine::Behavior::TypedParameterTupleInstance<Names, Ty...>> = {
    &Engine::Behavior::TypedParameterTupleInstance<Names, Ty...>::sMetaTablePtr,
    "<ParameterTuple>",
    Engine::Behavior::TypedParameterTupleInstance<Names, Ty...>::sMembers.data()
};

template <typename Names, typename... Ty>
const Engine::Reflect::MetaTable *Engine::Behavior::TypedParameterTupleInstance<Names, Ty...>::sMetaTablePtr = &table_instance<Engine::Behavior::TypedParameterTupleInstance<Names, Ty...>>;
