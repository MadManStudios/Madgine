#pragma once

#include "Meta/reflect/accessor.h"
#include "Meta/reflect/metatable.h"
#include "Meta/reflect/scopeptr.h"
#include "Meta/serialize/operations.h"
#include "Meta/serialize/streams/streamresult.h"

namespace Engine {
namespace Behavior {

    struct ParameterTupleBase {
        virtual size_t size() const = 0;
        virtual std::string_view name(size_t index) const = 0;
        virtual Reflect::ExtendedType type(size_t index) const = 0;

        virtual std::unique_ptr<ParameterTupleBase> clone() = 0;
        virtual Reflect::ScopePtr customScopePtr() = 0;

        virtual Serialize::StreamResult read(Serialize::CallerHierarchyFormattedSerializeStream in) = 0;
        virtual void write(Serialize::CallerHierarchyFormattedSerializeStream out) = 0;

        virtual ~ParameterTupleBase() = default;
    };

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

        Serialize::StreamResult read(Serialize::CallerHierarchyFormattedSerializeStream in) override
        {
            std::tuple<dependent_t<Serialize::StreamResult, Ty>...> results;
            [&]<size_t... Is>(auto_pack<Is...>) {
                ([&]() { std::get<Is>(results) = Serialize::read(in, std::get<Is>(this->mTuple), Names::template get<Is>.c_str()); }(), ...);
            }(index_pack_for<Ty...> {});

            return TupleUnpacker::accumulate(std::move(results), [](Serialize::StreamResult first, Serialize::StreamResult second) {
                    STREAM_PROPAGATE_ERROR(std::move(first));
                    return std::move(second); }, Serialize::StreamResult {});
        }

        void write(Serialize::CallerHierarchyFormattedSerializeStream out) override
        {
            [this, &out]<size_t... Is>(auto_pack<Is...>) {
                (Serialize::write(out, std::get<Is>(this->mTuple), Names::template get<Is>.c_str()), ...);
            }(index_pack_for<Ty...> {});
        }

        static const Reflect::MetaTable *sMetaTablePtr;

        template <size_t I>
        static Reflect::Result sGetter(const Reflect::Accessor *, Reflect::Value &retVal, const Reflect::Value &scope)
        {
            return invoke(retVal, [](TypedParameterTupleInstance &instance) -> decltype(auto) { return std::get<I>(instance.mTuple); }, scope);
        }

        template <size_t I, typename T>
        static Reflect::Result sSetter(const Reflect::Accessor *, const Reflect::Value &scope, const Reflect::Value &val)
        {
            return invoke([](TypedParameterTupleInstance &instance, T value) { std::get<I>(instance.mTuple) = std::move(value); }, scope, val);
        }

        static const constexpr auto sMembers = []<size_t... Is>(auto_pack<Is...>) constexpr -> std::array<Reflect::Accessor, sizeof...(Ty) + 1>
        {
            return { { { Names::template get<Is>.c_str(), nullptr, &sGetter<Is>, &sSetter<Is, Ty>, Reflect::toType<Ty>(), Concepts::InstanceOfA1<Ty, Named> ? Reflect::AccessorFlags_Named : Reflect::AccessorFlags_Default }...,
                {} } };
        }
        (index_pack_for<Ty...> {});
    };

    struct MADGINE_BEHAVIOR_EXPORT ParameterTuple {

        ParameterTuple() = default;
        ParameterTuple(const ParameterTuple &other)
            : mTuple(other.mTuple->clone())
        {
        }

        ParameterTuple(std::unique_ptr<ParameterTupleBase> tuple);

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

        Reflect::ScopePtr customScopePtr();

        template <typename... Ty>
        bool get(std::tuple<Ty...> &out) const
        {
            ParameterTupleInstance<Ty...> *instance = dynamic_cast<ParameterTupleInstance<Ty...> *>(mTuple.get());
            if (instance) {
                out = instance->mTuple;
            }
            return instance;
        }

        template <typename T>
        const T &get() const
        {
            return *static_cast<T *>(mTuple.get());
        }

        size_t size() const
        {
            return mTuple->size();
        }

        std::string_view name(size_t index) const
        {
            return mTuple->name(index);
        }

        Reflect::ExtendedType type(size_t index) const
        {
            return mTuple->type(index);
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

        friend Serialize::StreamResult tag_invoke(Serialize::apply_map_t, ParameterTuple &tuple, Serialize::CallerHierarchyFormattedSerializeStream in, bool success)
        {
            return {};
        }

        template <typename... Configs>
        friend void tag_invoke(Serialize::set_active_t<Configs...>, ParameterTuple &tuple, bool active, bool existenceChanged, const CallerHierarchyBasePtr &)
        {
        }

        std::unique_ptr<ParameterTupleBase> mTuple;
    };
}

namespace Serialize {
    template <>
    struct MADGINE_BEHAVIOR_EXPORT Operations<Behavior::ParameterTuple> {
        static StreamResult read(Serialize::CallerHierarchyFormattedSerializeStream in, Behavior::ParameterTuple &tuple, const char *name);
        static void write(Serialize::CallerHierarchyFormattedSerializeStream out, const Behavior::ParameterTuple &tuple, const char *name);

        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth);
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
