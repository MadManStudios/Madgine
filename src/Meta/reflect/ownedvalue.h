#pragma once

#include "../type/storage.h"
#include "../type/storageops_forward.h"

namespace Engine {
namespace Reflect {

    struct META_EXPORT OwnedValue {

        OwnedValue() = default;
        OwnedValue(const OwnedValue &) = default;
        // OwnedValue(OwnedValue &) = default;
        OwnedValue(OwnedValue &&) = default;
        OwnedValue(std::shared_ptr<Engine::Type::BaseStorage> ptr);

        OwnedValue &operator=(const OwnedValue &) = default;
        OwnedValue &operator=(OwnedValue &&) = default;

        template <typename T>
            requires(!std::same_as<std::decay_t<T>, OwnedValue> && !Concepts::InstanceOf<std::decay_t<T>, std::shared_ptr>)
        explicit OwnedValue(T &&t)
            : mValue(std::make_shared<Engine::Type::Storage<std::decay_t<T>>>(*storageOps<std::decay_t<T>>, std::forward<T>(t)))
        {
        }

        template <typename T>
            requires(!std::same_as<std::decay_t<T>, OwnedValue> && !Concepts::InstanceOf<std::decay_t<T>, std::shared_ptr>)
        OwnedValue &operator=(T &&t)
        {
            mValue = std::make_shared<Engine::Type::Storage<std::remove_reference_t<T>>>(*storageOps<std::decay_t<T>>, std::forward<T>(t));
            return *this;
        }

        std::string name() const;
        const MetaTable *type() const;

        bool operator==(const OwnedValue &other) const;

        void get(Value &retVal) const;
        Result set(const Value &value) const;

    private:
        std::shared_ptr<Engine::Type::BaseStorage> mValue;
    };

}
}