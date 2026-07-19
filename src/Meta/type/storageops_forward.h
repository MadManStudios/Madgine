#pragma once

DLL_IMPORT_VARIABLE(const Engine::Type::StorageOps, storageOps, SINGLE_ARG(Engine::Concepts::NoneOf<Engine::Void, Engine::Reflect::ScopePtr, Engine::Reflect::Value, Engine::Reflect::Result>));

namespace Engine {
namespace Type {

    META_EXPORT const StorageOps &resolveVariantStorageOps(const std::vector<const StorageOps *> &);

    template <typename... T>
    const StorageOps *sLocalVariantStorage = &resolveVariantStorageOps({ storageOps<T>... });

    template <typename T>
    constexpr const StorageOps **resolveStorageOps()
    {
        if constexpr (Concepts::InstanceOf<T, std::optional>) {
            return resolveStorageOps<std::variant<std::monostate, typename T::value_type>>();
        } else if constexpr (Concepts::InstanceOf<T, std::variant>) {
            return []<typename... Ts>(type_pack<Ts...>) {
                return &sLocalVariantStorage<Ts...>;
            }(typename Concepts::is_instance<T, std::variant>::argument_types {});
        } else {
            return &storageOps<T>;
        }
    }

}
}