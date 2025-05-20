#pragma once

#include "Generic/container/container_api.h"
#include "Generic/container/freelistcontainer.h"
#include "Meta/keyvalue/scopeptr.h"
#include "Meta/serialize/hierarchy/serializableunitptr.h"
#include "entitycomponentcollector.h"
#include "entitycomponentcontainer.h"
#include "entitycomponentlistbase.h"
#include "Meta/serialize/operations.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        DERIVE_FUNCTION(relocateComponent, const EntityComponentHandle<EntityComponentBase> &, Entity *)

        template <typename T>
        struct EntityComponentList : EntityComponentListBase {

            using Vector = container_api<typename replace<typename T::Container>::template type<T>>;

            Vector *operator->()
            {
                return &mData;
            }

            const Vector *operator->() const
            {
                return &mData;
            }

            T *get(const EntityComponentHandle<EntityComponentBase> &index) override final
            {
                return &mData.at(index.mIndex);
            }

            const T *get(const EntityComponentHandle<EntityComponentBase> &index) const override final
            {
                return &mData.at(index.mIndex);
            }

            ScopePtr getTyped(const EntityComponentHandle<EntityComponentBase> &index) override final
            {
                return &mData.at(index.mIndex);
            }

            Serialize::SerializableDataPtr getSerialized(const EntityComponentHandle<EntityComponentBase> &index) override final
            {
                return &mData.at(index.mIndex);
            }

            Serialize::SerializableDataConstPtr getSerialized(const EntityComponentHandle<EntityComponentBase> &index) const override final
            {
                return &mData.at(index.mIndex);
            }

            const Serialize::SerializeTable *serializeTable() const override final
            {
                return &::serializeTable<T>();
            }

            void init(const EntityComponentHandle<EntityComponentBase> &index) override final
            {
                if constexpr (requires { &T::init; })
                    mData.at(index.mIndex).init();
            }

            void finalize(const EntityComponentHandle<EntityComponentBase> &index) override final
            {
                if constexpr (requires { &T::finalize; })
                    mData.at(index.mIndex).finalize();
            }

            T *get(const EntityComponentHandle<T> &index)
            {
                return &mData.at(index.mIndex);
            }

            const T *get(const EntityComponentHandle<T> &index) const
            {
                return &mData.at(index.mIndex);
            }

            EntityComponentOwningHandle<EntityComponentBase> emplace(Entity *entity) override final
            {   
                typename Vector::iterator it = Engine::emplace(mData, mData.end(), entity);
                uint32_t index = container_traits<Vector>::toHandle(mData, it);
                return { { index, static_cast<uint32_t>(UniqueComponent::component_index<T>()) } };
            }

            void erase(const EntityComponentHandle<EntityComponentBase> &index) override final
            {
                auto it = container_traits<Vector>::toIterator(mData, index.mIndex);
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

            void setSynced(const EntityComponentHandle<EntityComponentBase> &index, bool synced) override final
            {
                Serialize::set_synced(mData.at(index.mIndex), synced);
            }

            void setActive(const EntityComponentHandle<EntityComponentBase> &index, bool active, bool existenceChanged) override final
            {
                Serialize::setActive(mData.at(index.mIndex), active, existenceChanged);
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