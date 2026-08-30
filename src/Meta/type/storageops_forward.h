#pragma once

#include "Generic/execution/concepts.h"

DLL_IMPORT_VARIABLE(const Engine::Type::StorageOps, storageOps, SINGLE_ARG(Engine::Concepts::NoneOf<Engine::Void, Engine::Reflect::ScopePtr, Engine::Reflect::Value, Engine::Reflect::Result>));

namespace Engine {
namespace Type {

    META_EXPORT const StorageOps &resolveVariantStorageOps(const std::vector<const StorageOps *> &);

    template <typename T>
    constexpr const StorageOps **resolveStorageOps();

    template <typename... T>
    const StorageOps *sLocalVariantStorage = &resolveVariantStorageOps({ *resolveStorageOps<T>()... });

    template <typename Ty>
    constexpr const StorageOps **resolveStorageOps()
    {
        using T = meta_decayed_t<Ty>;

        if constexpr (Concepts::InstanceOf<T, std::optional>) {
            return resolveStorageOps<std::variant<std::monostate, typename T::value_type>>();
        } else if constexpr (Concepts::InstanceOf<T, std::variant>) {
            return []<typename... Ts>(type_pack<Ts...>) {
                return &sLocalVariantStorage<Ts...>;
            }(typename Concepts::is_instance<T, std::variant>::argument_types {});
        } else if constexpr (Execution::AnyBinding<T>) {
            return resolveStorageOps<std::decay_t<typename T::type>>();
        } else {
            return &storageOps<std::remove_pointer_t<T>>;
        }
    }

}
}