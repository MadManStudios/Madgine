#pragma once

#include "behaviorcollector.h"

#include "parametertuple.h"

#include "Meta/keyvalue/valuetype.h"

#include "behavior.h"

namespace Engine {

struct NativeBehaviorInfo {
    virtual Behavior create(const ParameterTuple &args) const = 0;
    virtual ParameterTuple createParameters() const = 0;
    virtual std::string_view name() const = 0;
    virtual std::span<const ValueTypeDesc> parameterTypes() const = 0;
    virtual std::span<const ValueTypeDesc> resultTypes() const = 0;
    virtual std::span<const BindingDescriptor> bindings() const = 0;
};

DLL_IMPORT_VARIABLE(const NativeBehaviorInfo *, nativeBehaviorInfo, typename);

struct NativeBehaviorAnnotation {
    template <typename T>
    NativeBehaviorAnnotation(type_holder_t<T>)
        : mInfo(*nativeBehaviorInfo<T>)
    {
    }

    const NativeBehaviorInfo *mInfo;
};

}

DECLARE_NAMED_UNIQUE_COMPONENT(Engine, NativeBehavior, NativeBehaviorInfo, Engine::NativeBehaviorAnnotation)

namespace Engine {

template <typename T>
struct InputParameter {
};

template <typename T>
using is_parameter = is_instance<decayed_t<T>, InputParameter>;

template <typename T>
using is_value = std::negation<is_parameter<T>>;

template <typename T>
struct get_type {
    using type = T;
};

template <typename T>
struct get_type<InputParameter<T>> {
    using type = T;
};

template <typename T>
using get_type_t = typename get_type<T>::type;

template <typename T, auto Factory, typename... Arguments>
struct NativeBehavior : NativeBehaviorComponent<T, NativeBehaviorInfo> {

    using argument_types = type_pack<Arguments...>;
    using parameter_arguments = typename argument_types::template filter<is_parameter>;
    using value_arguments = typename argument_types::template filter<is_value>;
    using parameter_argument_tuple = typename parameter_arguments::template transform<get_type_t>::template instantiate<std::tuple>;

    template <uint32_t I>
    static auto buildArgs(const std::tuple<> &parameters, type_pack<> args)
    {
        return std::make_tuple();
    }

    template <uint32_t I, typename... Vs, typename U, typename... Us>
    static auto buildArgs(std::tuple<Vs...> &&parameters, type_pack<U, Us...> args)
    {
        if constexpr (is_value<U>::value) {
            return std::tuple_cat(
                std::make_tuple(U {}),
                buildArgs<I>(std::move(parameters), type_pack<Us...> {}));
        } else {
            return TupleUnpacker::prepend<decayed_t<first_t<Vs...>>>(
                std::get<0>(std::move(parameters)),
                buildArgs<I>(TupleUnpacker::popFront(std::move(parameters)), type_pack<Us...> {}));
        }
    }

    static auto buildSender(parameter_argument_tuple &&parameters)
    {
        return TupleUnpacker::invokeFromTuple(Factory, buildArgs<0>(std::move(parameters), argument_types {}));
    }

    using Sender = decltype(buildSender(std::declval<parameter_argument_tuple>()));

    NativeBehavior(std::string_view name)
        : mName(name)
    {
    }

    virtual Behavior create(const ParameterTuple &args) const override
    {
        parameter_argument_tuple parameters;

        if (!args.get(parameters))
            throw 0;

        return buildSender(std::move(parameters));
    };

    virtual ParameterTuple createParameters() const override
    {
        return parameter_argument_tuple {};
    }

    virtual std::string_view name() const override
    {
        return mName;
    }

    static constexpr auto sParameterTypes = []() {
        if constexpr (std::same_as<parameter_arguments, type_pack<>>) {
            return std::span<const ValueTypeDesc> {};
        } else {
            return []<typename... P>(type_pack<P...>)
            {
                return std::array<ValueTypeDesc, sizeof...(P)> {
                    toValueTypeDesc<P>()...
                };
            }
            (typename parameter_arguments::template transform<get_type_t> {});
        }
    }();
    virtual std::span<const ValueTypeDesc> parameterTypes() const override
    {
        return sParameterTypes;
    }

    static constexpr auto sResultTypes = []() {
        if constexpr (std::same_as<typename Sender::template value_types<type_pack>, type_pack<>>) {
            return std::span<const ValueTypeDesc> {};
        } else if constexpr (std::same_as<typename Sender::template value_types<type_pack>, type_pack<ArgumentList>>) {
            return std::span<const ValueTypeDesc> {};
        } else {
            return []<typename... P>(type_pack<P...>)
            {
                return std::array<ValueTypeDesc, sizeof...(P)> {
                    toValueTypeDesc<P>()...
                };
            }
            (typename Sender::template value_types<type_pack> {});
        }
    }();
    virtual std::span<const ValueTypeDesc> resultTypes() const override
    {
        return sResultTypes;
    }

    static constexpr auto sBindings = []() {
        return std::span<const BindingDescriptor> {};
    }();
    std::span<const BindingDescriptor> bindings() const override {
        return sBindings;
    }

    std::string_view mName;
};

struct NativeBehaviorFactory : BehaviorFactory<NativeBehaviorFactory> {
    std::vector<std::string_view> names() const override;
    UniqueOpaquePtr load(std::string_view name) const override;
    Threading::TaskFuture<bool> state(const UniqueOpaquePtr &handle) const override;
    void release(UniqueOpaquePtr &ptr) const override;
    std::string_view name(const UniqueOpaquePtr &handle) const override;
    Behavior create(const UniqueOpaquePtr &handle, const ParameterTuple &args) const override;
    Threading::TaskFuture<ParameterTuple> createParameters(const UniqueOpaquePtr &handle) const override;
    ParameterTuple createDummyParameters(const UniqueOpaquePtr &handle) const override;
    std::vector<ValueTypeDesc> parameterTypes(const UniqueOpaquePtr &handle) const override;
    std::vector<ValueTypeDesc> resultTypes(const UniqueOpaquePtr &handle) const override;
    std::vector<BindingDescriptor> bindings(const UniqueOpaquePtr &handle) const override;
};

}

DECLARE_BEHAVIOR_FACTORY(Engine::NativeBehaviorFactory)
REGISTER_TYPE(Engine::NativeBehaviorInfo)

#define NATIVE_BEHAVIOR_DECLARATION(Name) \
    struct Name##NativeBehavior;          \
    REGISTER_TYPE(Name##NativeBehavior)

#define NATIVE_BEHAVIOR(Name, Sender, ...)                                                                             \
    struct Name##Linkage {                                                                                             \
        template <typename... Args>                                                                                    \
        auto operator()(Args &&...args) const                                                                          \
        {                                                                                                              \
            return Sender(std::forward<Args>(args)...);                                                                \
        }                                                                                                              \
    };                                                                                                                 \
                                                                                                                       \
    using Name##NativeBehaviorType = Engine::NativeBehavior < Name##NativeBehavior, Name##Linkage                      \
    {                                                                                                                  \
    }                                                                                                                  \
    __VA_OPT__(, )                                                                                                     \
    __VA_ARGS__                                                                                                        \
        > ;                                                                                                            \
                                                                                                                       \
    struct Name##NativeBehavior : Name##NativeBehaviorType {                                                           \
        using Name##NativeBehaviorType::Name##NativeBehaviorType;                                                      \
    };                                                                                                                 \
                                                                                                                       \
    static const Name##NativeBehavior Name##Info { #Name };                                                   \
                                                                                                                       \
    DLL_EXPORT_VARIABLE(, const Engine::NativeBehaviorInfo *, Engine::, nativeBehaviorInfo, &Name##Info, Name##NativeBehavior) \
                                                                                                                       \
        NAMED_UNIQUECOMPONENT(Name, Name##NativeBehavior)
