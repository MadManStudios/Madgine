#pragma once

#include "Generic/containers/container_api.h"
#include "Generic/containers/freelistcontainer.h"

#include "Meta/reflect/scopeptr.h"
#include "Meta/serialize/hierarchy/serializableunitptr.h"
#include "Meta/serialize/operations.h"

#include "entitycomponentcollector.h"
#include "entitycomponentcontainer.h"
#include "entitycomponentlistbase.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        template <typename T>
        struct EntityComponentList : EntityComponentListBase {

            using Vector = Containers::container_api<typename replace<typename T::Container>::template type<T>>;

            Vector *operator->()
            {
                return &mData;
            }

            const Vector *operator->() const
            {
                return &mData;
            }

            Reflect::ScopePtr getTyped(EntityComponentBase &comp) override final
            {
                return static_cast<T *>(&comp);
            }

            Serialize::SerializableDataPtr getSerialized(EntityComponentBase &comp) override final
            {
                return static_cast<T *>(&comp);
            }

            Serialize::SerializableDataConstPtr getSerialized(const EntityComponentBase &comp) const override final
            {
                return static_cast<const T *>(&comp);
            }

            const Serialize::SerializeTable *serializeTable() const override final
            {
                return &::serializeTable<T>();
            }

            void init(EntityComponentBase &comp) override final
            {
                if constexpr (requires { &T::init; })
                    static_cast<T &>(comp).init();
            }

            void finalize(EntityComponentBase &comp) override final
            {
                if constexpr (requires { &T::finalize; })
                    static_cast<T &>(comp).finalize();
            }

            EntityComponentBase &emplace(Entity &entity) override final
            {
                typename Vector::iterator it = Containers::emplace(mData, mData.end(), entity);
                return *it;
            }

            void erase(EntityComponentBase &comp) override final
            {
                auto it = std::ranges::find_if(mData, [&comp](auto &element) { return &element == &comp; });
                mData.erase(it);
            }

            bool empty() override final
            {
                return mData.empty();
            }

            void clear() override final
            {
                mData.clear();
            }

            size_t size() const override final
            {
                return mData.size();
            }

            void setSynced(EntityComponentBase &comp, bool synced) override final
            {
                Serialize::set_synced(static_cast<T &>(comp), synced);
            }

            void setActive(EntityComponentBase &comp, bool active, bool existenceChanged) override final
            {
                Serialize::set_active<>(static_cast<T &>(comp), active, existenceChanged);
            }

            auto begin()
            {
                return mData.begin();
            }

            auto end()
            {
                return mData.end();
            }

            auto &data()
            {
                return mData;
            }

            T &front()
            {
                return mData.front();
            }

            T &operator[](size_t index)
            {
                return mData[index];
            }

            Vector mData;
        };

    }
}
}