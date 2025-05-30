#pragma once

#include "behaviorcollector.h"

#include "parametertuple.h"

#include "Meta/keyvalue/valuetype.h"

#include "behavior.h"

namespace Engine {

struct NativeBehaviorInfo {
    virtual Behavior create(const ParameterTuple &args, std::vector<Behavior> behaviors) const = 0;
    virtual ParameterTuple createParameters() const = 0;
    virtual std::string_view name() const = 0;
    virtual std::span<const ValueTypeDesc> parameterTypes() const = 0;
    virtual std::span<const ValueTypeDesc> resultTypes() const = 0;
    virtual std::span<const BindingDescriptor> bindings() const = 0;
    virtual size_t subBehaviorCount() const = 0;
};

DLL_IMPORT_VARIABLE(const NativeBehaviorInfo *, nativeBehaviorInfo, typename);

struct NativeBehaviorAnnotation {
    template <typename T, typename ActualType>
    NativeBehaviorAnnotation(type_holder_t<T>, type_holder_t<ActualType>)
        : mInfo(*nativeBehaviorInfo<T>)
    {
    }

    const NativeBehaviorInfo *mInfo;
};

}

DECLARE_NAMED_UNIQUE_COMPONENT(Engine, NativeBehavior, NativeBehaviorInfo, Engine::NativeBehaviorAnnotation)

namespace Engine {

template <fixed_string Name, typename T>
struct InputParameter {
};

struct SubBehavior {
};

template <typename T>
using is_parameter = is_instance_auto1<decayed_t<T>, InputParameter>;

template <typename T>
using is_sub_behavior = std::is_same<T, SubBehavior>;

template <typename T>
using is_value = std::negation<std::disjunction<is_parameter<T>, is_sub_behavior<T>>>;

template <typename T>
struct get_type;

template <typename T, auto Name>
struct get_type<InputParameter<Name, T>> {
    using type = T;
};

template <typename T>
using get_type_t = typename get_type<T>::type;

template <typename T>
struct get_name;

template <typename T, auto Name>
struct get_name<InputParameter<Name, T>> {
    static constexpr auto value = Name;
};

template <typename T, auto Factory, typename... Arguments>
struct NativeBehavior : NativeBehaviorComponent<T, NativeBehaviorInfo> {

    using argument_types = type_pack<Arguments...>;
    using parameter_arguments = typename argument_types::template filter<is_parameter>;
    using subbehavior_arguments = typename argument_types::template filter<is_sub_behavior>;
    using value_arguments = typename argument_types::template filter<is_value>;
    using parameter_argument_tuple = typename parameter_arguments::template transform<get_type_t>::template instantiate<std::tuple>;
    using parameter_argument_names = typename parameter_arguments::template value_transform<get_name>;

    template <uint32_t I>
    static auto buildArgs(const std::tuple<> &parameters, type_pack<> args, std::vector<Behavior> behaviors)
    {
        assert(behaviors.empty());
        return std::make_tuple();
    }

    template <uint32_t I, typename... Vs, typename U, typename... Us>
    static auto buildArgs(std::tuple<Vs...> &&parameters, type_pack<U, Us...> args, std::vector<Behavior> behaviors)
    {
        if constexpr (is_value<U>::value) {
            return std::tuple_cat(
                std::make_tuple(U {}),
                buildArgs<I>(std::move(parameters), type_pack<Us...> {}, std::move(behaviors)));
        } else if constexpr (is_sub_behavior<U>::value) {
            Behavior behavior = std::move(behaviors.front());
            behaviors.erase(behaviors.begin());
            return std::tuple_cat(
                std::make_tuple(std::move(behavior)),
                buildArgs<I>(std::move(parameters), type_pack<Us...> {}, std::move(behaviors)));
        } else {
            return TupleUnpacker::prepend<decayed_t<first_t<Vs...>>>(
                std::get<0>(std::move(parameters)),
                buildArgs<I>(TupleUnpacker::popFront(std::move(parameters)), type_pack<Us...> {}, std::move(behaviors)));
        }
    }

    static auto buildSender(parameter_argument_tuple &&parameters, std::vector<Behavior> behaviors)
    {
        return TupleUnpacker::invokeFromTuple(Factory, buildArgs<0>(std::move(parameters), argument_types {}, std::move(behaviors)));
    }

    using Sender = decltype(buildSender(std::declval<parameter_argument_tuple>(), std::declval<std::vector<Behavior>>()));

    NativeBehavior(std::string_view name)
        : mName(name)
    {
    }

    virtual Behavior create(const ParameterTuple &args, std::vector<Behavior> behaviors) const override
    {
        parameter_argument_tuple parameters;

        if (!args.get(parameters))
            throw 0;

        return buildSender(std::move(parameters), std::move(behaviors));
    };

    virtual ParameterTuple createParameters() const override
    {
        return { parameter_argument_tuple {}, parameter_argument_names {} };
    }

    virtual std::string_view name() const override
    {
        return mName;
    }

    static constexpr auto sParameterTypes = []() {
        if constexpr (std::same_as<parameter_arguments, type_pack<>>) {
            return std::span<const ValueTypeDesc> {};
        } else {
            return []<typename... P>(type_pack<P...>) {
                return std::array<ValueTypeDesc, sizeof...(P)> {
                    toValueTypeDesc<P>()...
                };
            }(typename parameter_arguments::template transform<get_type_t> {});
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
            return []<typename... P>(type_pack<P...>) {
                return std::array<ValueTypeDesc, sizeof...(P)> {
                    toValueTypeDesc<P>()...
                };
            }(typename Sender::template value_types<type_pack> {});
        }
    }();
    virtual std::span<const ValueTypeDesc> resultTypes() const override
    {
        return sResultTypes;
    }

    static constexpr auto sBindings = []() {
        return std::span<const BindingDescriptor> {};
    }();
    std::span<const BindingDescriptor> bindings() const override
    {
        return sBindings;
    }

    size_t subBehaviorCount() const override
    {
        return subbehavior_arguments::size;
    }

    std::string_view mName;
};

struct NativeBehaviorFactory : BehaviorFactory<NativeBehaviorFactory> {
    std::vector<std::string_view> names() const override;
    UniqueOpaquePtr load(std::string_view name) const override;
    Threading::TaskFuture<bool> state(const UniqueOpaquePtr &handle) const override;
    void release(UniqueOpaquePtr &ptr) const override;
    std::string_view name(const UniqueOpaquePtr &handle) const override;
    Behavior create(const UniqueOpaquePtr &handle, const ParameterTuple &args, std::vector<Behavior> behaviors) const override;
    Threading::TaskFuture<ParameterTuple> createParameters(const UniqueOpaquePtr &handle) const override;
    ParameterTuple createDummyParameters(const UniqueOpaquePtr &handle) const override;
    std::vector<ValueTypeDesc> parameterTypes(const UniqueOpaquePtr &handle) const override;
    std::vector<ValueTypeDesc> resultTypes(const UniqueOpaquePtr &handle) const override;
    std::vector<BindingDescriptor> bindings(const UniqueOpaquePtr &handle) const override;
    size_t subBehaviorCount(const UniqueOpaquePtr &handle) const override;
};

}

DECLARE_BEHAVIOR_FACTORY(Engine::NativeBehaviorFactory)
REGISTER_TYPE(Engine::NativeBehaviorInfo)

#define NATIVE_BEHAVIOR_DECLARATION(Name) \
    struct Name##NativeBehavior;          \
    REGISTER_TYPE(Name##NativeBehavior)

#define NATIVE_BEHAVIOR(Name, Sender, ...)                                                                                     \
    struct Name##Linkage {                                                                                                     \
        template <typename... Args>                                                                                            \
        auto operator()(Args &&...args) const                                                                                  \
        {                                                                                                                      \
            return Sender(std::forward<Args>(args)...);                                                                        \
        }                                                                                                                      \
    };                                                                                                                         \
                                                                                                                               \
    using Name##NativeBehaviorType = Engine::NativeBehavior < Name##NativeBehavior, Name##Linkage                              \
    {                                                                                                                          \
    }                                                                                                                          \
    __VA_OPT__(, )                                                                                                             \
    __VA_ARGS__                                                                                                                \
        > ;                                                                                                                    \
                                                                                                                               \
    struct Name##NativeBehavior : Name##NativeBehaviorType {                                                                   \
        using Name##NativeBehaviorType::Name##NativeBehaviorType;                                                              \
    };                                                                                                                         \
                                                                                                                               \
    static const Name##NativeBehavior Name##Info { #Name };                                                                    \
                                                                                                                               \
    DLL_EXPORT_VARIABLE(, const Engine::NativeBehaviorInfo *, Engine::, nativeBehaviorInfo, &Name##Info, Name##NativeBehavior) \
                                                                                                                               \
    NAMED_UNIQUECOMPONENT(Name, Name##NativeBehavior)
