#pragma once

#include "Generic/execution/algorithm.h"

#include "behavior.h"
#include "behaviorcollector.h"
#include "behaviordescriptor.h"
#include "parametertuple.h"
#include "typedparametertuple.h"

namespace Engine {
namespace Behavior {

    struct NativeBehaviorInfo {
        virtual Behavior create(const Reflect::ArgumentList &args, std::vector<Behavior> behaviors) const = 0;
        virtual ParameterTuple createParameters() const = 0;
        virtual const BehaviorDescriptor &descriptor() const = 0;
        virtual std::string_view name() const = 0;
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
}

DECLARE_NAMED_UNIQUE_COMPONENT(Engine::Behavior, NativeBehavior, NativeBehaviorInfo, Engine::Behavior::NativeBehaviorAnnotation)

namespace Engine {
namespace Behavior {

    template <fixed_string Name, typename T>
    struct InputParameter {
    };

    struct SubBehavior {
    };

    template <typename T>
    using is_parameter = Concepts::is_instance_auto1<meta_decayed_t<T>, InputParameter>;

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

    template <>
    struct get_type<SubBehavior> {
        using type = Behavior;
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

        template <uint32_t I, typename... Args>
        static auto buildSenderHelper(const Reflect::ArgumentList &parameters, type_pack<> types, std::vector<Behavior> behaviors, Args &&...args)
        {
            assert(behaviors.empty());
            return Factory(std::forward<Args>(args)...);
        }

        template <uint32_t I, typename V, typename... Vs, typename... Args>
        static Behavior buildSenderHelper(const Reflect::ArgumentList &parameters, type_pack<V, Vs...> types, std::vector<Behavior> behaviors, Args &&...args)
        {
            if constexpr (is_value<V>::value) {
                return buildSenderHelper<I>(std::move(parameters), type_pack<Vs...> {}, std::move(behaviors), std::forward<Args>(args)..., V {});
            } else if constexpr (is_sub_behavior<V>::value) {
                Behavior behavior = std::move(behaviors.front());
                behaviors.erase(behaviors.begin());
                return buildSenderHelper<I>(std::move(parameters), type_pack<Vs...> {}, std::move(behaviors), std::forward<Args>(args)..., std::move(behavior));
            } else {
                Behavior result;
                if (Reflect::Result error = Reflect::call([&](const get_type_t<V> &value) {
                        result = buildSenderHelper<I + 1>(parameters, type_pack<Vs...> {}, std::move(behaviors), std::forward<Args>(args)..., value);
                        return Reflect::Result {};
                    },
                        parameters.at(I))) {
                    result = Execution::just_error(std::move(*error.mError));
                }
                return result;
            }
        }

        static Behavior buildSender(const Reflect::ArgumentList &args, std::vector<Behavior> behaviors)
        {
            return buildSenderHelper<0>(args, argument_types {}, std::move(behaviors));
        }

        using Sender = std::invoke_result_t<decltype(Factory), get_type_t<Arguments>...>;

        NativeBehavior(std::string_view name)
            : mName(name)
        {
        }

        Behavior create(const Reflect::ArgumentList &args, std::vector<Behavior> behaviors) const override
        {
            return buildSender(args, std::move(behaviors));
        };

        virtual ParameterTuple createParameters() const override
        {
            return { parameter_argument_tuple {}, parameter_argument_names {} };
        }

        static constexpr auto sParameterTypes = []() {
            if constexpr (std::same_as<parameter_arguments, type_pack<>>) {
                return std::span<const BehaviorDescriptor::Parameter> {};
            } else {
                return []<typename... P>(type_pack<P...>) {
                    return std::array<BehaviorDescriptor::Parameter, sizeof...(P)> { { { get_name<P>::value.c_str(), Type::resolveStorageOps<get_type_t<P>>() }... } };
                }(parameter_arguments {});
            }
        }();

        static constexpr auto sResultTypes = []() {
            if constexpr (std::same_as<typename Sender::template value_types<type_pack>, type_pack<>>) {
                return std::span<const Reflect::ExtendedType> {};
            } else if constexpr (std::same_as<typename Sender::template value_types<type_pack>, type_pack<Reflect::ArgumentList>>) {
                return std::span<const Reflect::ExtendedType> {};
            } else {
                return []<typename... P>(type_pack<P...>) {
                    return std::array<Reflect::ExtendedType, sizeof...(P)> {
                        Reflect::toType<P>()...
                    };
                }(typename Sender::template value_types<type_pack> {});
            }
        }();

        const BehaviorDescriptor &descriptor() const override
        {
            static constexpr BehaviorDescriptor sDescriptor {
                sParameterTypes,
                sResultTypes,
                subbehavior_arguments::size
            };

            return sDescriptor;
        }

        std::string_view name() const override
        {
            return mName;
        }

        std::string_view mName;
    };

    struct NativeBehaviorFactory : BehaviorFactory<NativeBehaviorFactory> {
        std::vector<std::string_view> names() const override;
        UniqueOpaquePtr load(std::string_view name) const override;
        Threading::TaskFuture<bool> state(const UniqueOpaquePtr &handle) const override;
        void release(UniqueOpaquePtr &ptr) const override;
        std::string_view name(const UniqueOpaquePtr &handle) const override;
        Behavior create(const UniqueOpaquePtr &handle, const Reflect::ArgumentList &args, std::vector<Behavior> behaviors) const override;
        ParameterTuple createParameters(const UniqueOpaquePtr &handle) const override;
        const BehaviorDescriptor &descriptor(const UniqueOpaquePtr &handle) const override;
    };

}
}

#define NATIVE_BEHAVIOR(Name, Sender, ...)                                                                                                                                               \
    namespace __behavior_impl__ {                                                                                                                                                        \
        struct Name##NativeBehavior;                                                                                                                                                     \
        struct Name##Linkage {                                                                                                                                                           \
            template <typename... Args>                                                                                                                                                  \
            auto operator()(Args &&...args) const                                                                                                                                        \
            {                                                                                                                                                                            \
                return Sender(std::forward<Args>(args)...);                                                                                                                              \
            }                                                                                                                                                                            \
        };                                                                                                                                                                               \
                                                                                                                                                                                         \
        using Name##NativeBehaviorType = Engine::Behavior::NativeBehavior < Name##NativeBehavior, Name##Linkage                                                                          \
        {                                                                                                                                                                                \
        }                                                                                                                                                                                \
        __VA_OPT__(, )                                                                                                                                                                   \
        __VA_ARGS__                                                                                                                                                                      \
            > ;                                                                                                                                                                          \
                                                                                                                                                                                         \
        struct Name##NativeBehavior : Name##NativeBehaviorType {                                                                                                                         \
            using Name##NativeBehaviorType::Name##NativeBehaviorType;                                                                                                                    \
        };                                                                                                                                                                               \
                                                                                                                                                                                         \
        static const Name##NativeBehavior Name##Info { #Name };                                                                                                                          \
    }                                                                                                                                                                                    \
                                                                                                                                                                                         \
    DLL_EXPORT_VARIABLE(, const Engine::Behavior::NativeBehaviorInfo *, Engine::Behavior::, nativeBehaviorInfo, &__behavior_impl__::Name##Info, __behavior_impl__::Name##NativeBehavior) \
                                                                                                                                                                                         \
    NAMED_UNIQUECOMPONENT(Name, __behavior_impl__::Name##NativeBehavior)
