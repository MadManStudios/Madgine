#pragma once

#include "Generic/fixed_string.h"
#include "Generic/guard.h"
#include "Generic/linestruct.h"

#include "storageops.h"

namespace Engine {
namespace Type {

    template <typename T>
    struct Derived {
    };

    template <typename T>
    struct Variadic {
        using type = T;
    };

    namespace __Type_impl__ {

        struct StorageOpsCtorTag;

        template <typename T>
        struct ConstructorParameter {
            using type = T;
        };

        template <typename T>
        struct ConstructorParameter<Variadic<T>> {
        };

        template <typename T>
        struct ConstructorParameter<Derived<T>> {
            using type = Reflect::ScopePtr;
        };

        template <typename T, typename... Args>
            requires(!(Concepts::InstanceOf<Args, Variadic> || ...))
        static constexpr Constructor ctor(type_pack<Args...>)
        {
            return {
                [](const StorageOps &, const Reflect::ArgumentList &args) {
                    return true;
                },
                []<size_t... Is>(std::index_sequence<Is...>) {
                    return [](const StorageOps &type, BaseStorage &out, const Reflect::ArgumentList &args, size_t inlineSize) -> Reflect::Result {
                        return Reflect::invoke_free([&](ConstructorParameter<Args>::type... arg) {
                            if (inlineSize >= sizeof(T)) {
                                new (&static_cast<Storage<T> &>(out).mObject) T(std::forward<typename ConstructorParameter<Args>::type>(arg)...);
                            } else if (inlineSize > 0) {
                                new (&static_cast<AllocationStorage &>(out).mAllocation) AllocationPtr { new Storage<T>(type, std::forward<typename ConstructorParameter<Args>::type>(arg)...) };
                            } else {
                                static_cast<AllocationStorage &>(out).mAllocation = AllocationPtr { new Storage<T>(type, std::forward<typename ConstructorParameter<Args>::type>(arg)...) };
                            } }, {},
                            getArgument(args, Is)...);
                    };
                }(std::index_sequence_for<Args...> {})
            };
        }

        template <typename T>
        static constexpr Constructor ctor(type_pack<void>)
        {
            return {};
        }

        template <typename VariadicArg, typename CB>
        static Reflect::Result variadic_ctor(std::vector<VariadicArg> variadicArgs, const Reflect::ArgumentList &args, size_t i, CB &&cb)
        {
            if (i < argumentCount(args)) {
                return call([&](VariadicArg arg) {
                    variadicArgs.emplace_back(std::forward<VariadicArg>(arg));
                    return variadic_ctor(std::move(variadicArgs), args, i + 1, std::forward<CB>(cb));
                },
                    getArgument(args, i));
            } else {
                return cb(std::move(variadicArgs));
            }
        }

        template <typename T, typename Arg, typename... Args>
            requires Concepts::InstanceOf<last_t<Arg, Args...>, Variadic>
        static constexpr Constructor ctor(type_pack<Arg, Args...>)
        {
            using Variadic = last_t<Arg, Args...>;
            using VariadicArg = ConstructorParameter<typename Variadic::type>::type;

            return {
                [](const StorageOps &, const Reflect::ArgumentList &args) {
                    return true;
                },
                []<size_t... Is>(std::index_sequence<Is...>) {
                    return [](const StorageOps &type, BaseStorage &out, const Reflect::ArgumentList &args, size_t inlineSize) -> Reflect::Result {
                        return variadic_ctor<VariadicArg>({}, args, sizeof...(Is),
                            [&](std::vector<VariadicArg> variadicArgs) { return Reflect::invoke_free([&](ConstructorParameter<Args>::type... arg) {
                                                                             if (inlineSize >= sizeof(T)) {
                                                                                 new (&static_cast<Storage<T> &>(out).mObject) T(std::forward<typename ConstructorParameter<Args>::type>(arg)..., std::move(variadicArgs));
                                                                             } else if (inlineSize > 0) {
                                                                                 new (&static_cast<AllocationStorage &>(out).mAllocation) AllocationPtr { new Storage<T>(type, std::forward<typename ConstructorParameter<Args>::type>(arg)..., std::move(variadicArgs)) };
                                                                             } else {
                                                                                 static_cast<AllocationStorage &>(out).mAllocation = AllocationPtr { new Storage<T>(type, std::forward<typename ConstructorParameter<Args>::type>(arg)..., std::move(variadicArgs)) };
                                                                             } }, {},
                                                                             getArgument(args, Is)...); });
                    };
                }(std::index_sequence_for<Args...> {})
            };
        }

        template <typename T, auto f, typename... Args>
            requires(!(Concepts::InstanceOf<Args, Variadic> || ...))
        static constexpr Constructor factory(type_pack<Args...>)
        {
            return {
                [](const StorageOps &, const Reflect::ArgumentList &args) {
                    return true;
                },
                []<size_t... Is>(std::index_sequence<Is...>) {
                    return [](const StorageOps &type, BaseStorage &out, const Reflect::ArgumentList &args, size_t inlineSize) -> Reflect::Result {
                        return Reflect::invoke_free([&](ConstructorParameter<Args>::type... arg) {
                            if (inlineSize >= sizeof(T)) {
                                new (&static_cast<Storage<T> &>(out).mObject) T(f(std::forward<typename ConstructorParameter<Args>::type>(arg)...));
                            } else if (inlineSize > 0) {
                                new (&static_cast<AllocationStorage &>(out).mAllocation) AllocationPtr { new Storage<T>(type, f(std::forward<typename ConstructorParameter<Args>::type>(arg)...)) };
                            } else {
                                static_cast<AllocationStorage &>(out).mAllocation = AllocationPtr { new Storage<T>(type, f(std::forward<typename ConstructorParameter<Args>::type>(arg)...)) };
                            } }, {},
                            getArgument(args, Is)...);
                    };
                }(std::index_sequence_for<Args...> {})
            };
        }

    }
}
}

#define STORAGEOPS_BEGIN(T, ActualType) \
    STORAGEOPS_BEGIN_EX(, SINGLE_ARG(T), ActualType)

#define STORAGEOPS_BEGIN_EX(Idx, T, ActualType)                                                         \
    namespace Engine {                                                                                  \
        namespace __generic_impl__ {                                                                    \
            START_STRUCT(Type::__Type_impl__::StorageOpsCtorTag, Idx)                                   \
            {                                                                                           \
                using Ty = ActualType;                                                                  \
                constexpr const Type::Constructor *data(const Type::Constructor *p) const { return p; } \
            };                                                                                          \
        }                                                                                               \
    }

#define CONSTRUCTOR_EX(Idx, ...)                                                                    \
    namespace Engine {                                                                              \
        namespace __generic_impl__ {                                                                \
            LINE_STRUCT(Type::__Type_impl__::StorageOpsCtorTag, Idx)                                \
            {                                                                                       \
                constexpr const Type::Constructor *data(const Type::Constructor *) const            \
                {                                                                                   \
                    return BASE_STRUCT(Type::__Type_impl__::StorageOpsCtorTag, Idx)::data(&mData);  \
                }                                                                                   \
                Type::Constructor mData = Type::__Type_impl__::ctor<Ty>(type_pack<__VA_ARGS__> {}); \
            };                                                                                      \
        }                                                                                           \
    }

#define FACTORY_EX(Idx, f, ...)                                                                           \
    namespace Engine {                                                                                    \
        namespace __generic_impl__ {                                                                      \
            LINE_STRUCT(Type::__Type_impl__::StorageOpsCtorTag, Idx)                                      \
            {                                                                                             \
                constexpr const Type::Constructor *data(const Type::Constructor *) const                  \
                {                                                                                         \
                    return BASE_STRUCT(Type::__Type_impl__::StorageOpsCtorTag, Idx)::data(&mData);        \
                }                                                                                         \
                Type::Constructor mData = Type::__Type_impl__::factory<Ty, f>(type_pack<__VA_ARGS__> {}); \
            };                                                                                            \
        }                                                                                                 \
    }

#define STORAGEOPS_VALUE_BEGIN(T) \
    STORAGEOPS_BEGIN(T, T)

#define CONSTRUCTOR(...) CONSTRUCTOR_EX(, __VA_ARGS__)

#define STORAGEOPS_END(T) \
    STORAGEOPS_END_EX(, SINGLE_ARG(T))

#define STORAGEOPS_END_EX(Idx, T) \
    CONSTRUCTOR_EX(Idx, void)     \
    STORAGEOPS_INSTANTIATION(Idx, T)

#define STORAGEOPS_INSTANTIATION(Idx, T)                                                                \
    namespace Meta_##T                                                                                  \
    {                                                                                                   \
        static constexpr GET_STRUCT(::Engine::Type::__Type_impl__::StorageOpsCtorTag, Idx) sCtors = {}; \
    }                                                                                                   \
    DLL_EXPORT_VARIABLE(constexpr, const ::Engine::Type::StorageOps, , storageOps, SINGLE_ARG({ &storageOps<T>, #T, ::Engine::type_holder<GET_STRUCT(::Engine::Type::__Type_impl__::StorageOpsCtorTag, Idx)::Ty>, Meta_##T::sCtors.data(nullptr) }), T);
