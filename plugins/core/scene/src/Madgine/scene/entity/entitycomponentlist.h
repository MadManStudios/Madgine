#pragma once

#include "Generic/container/container_api.h"
#include "Generic/container/freelistcontainer.h"
#include "Meta/keyvalue/typedscopeptr.h"
#include "Meta/serialize/hierarchy/serializableunitptr.h"
#include "entitycomponentcollector.h"
#include "entitycomponentcontainer.h"
#include "entitycomponentlistbase.h"
#include "Meta/serialize/operations.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        DERIVE_FUNCTION(init, Entity *)
        DERIVE_FUNCTION(finalize, Entity *)
        DERIVE_FUNCTION(relocateComponent, const EntityComponentHandle<EntityComponentBase> &, Entity *)

        template <typename T>
        struct EntityComponentList : EntityComponentListComponent<EntityComponentList<T>> {

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
                return &(get_component(mData.at(index.mIndex)));
            }

            const T *get(const EntityComponentHandle<EntityComponentBase> &index) const override final
            {
                return &(get_component(mData.at(index.mIndex)));
            }

            ScopePtr getTyped(const EntityComponentHandle<EntityComponentBase> &index) override final
            {
                return &(get_component(mData.at(index.mIndex)));
            }

            Serialize::SerializableDataPtr getSerialized(const EntityComponentHandle<EntityComponentBase> &index) override final
            {
                return &(get_component(mData.at(index.mIndex)));
            }

            Serialize::SerializableDataConstPtr getSerialized(const EntityComponentHandle<EntityComponentBase> &index) const override final
            {
                return &(get_component(mData.at(index.mIndex)));
            }

            const Serialize::SerializeTable *serializeTable() const override final
            {
                return &::serializeTable<T>();
            }

            void init(const EntityComponentHandle<EntityComponentBase> &index, Entity *entity) override final
            {
                if constexpr (has_function_init_v<T>)
                    get_component(mData.at(index.mIndex)).init(entity);
            }

            void finalize(const EntityComponentHandle<EntityComponentBase> &index, Entity *entity) override final
            {
                if constexpr (has_function_finalize_v<T>)
                    get_component(mData.at(index.mIndex)).finalize(entity);
            }

            T *get(const EntityComponentHandle<T> &index)
            {
                return &(get_component(mData.at(index.mIndex)));
            }

            const T *get(const EntityComponentHandle<T> &index) const
            {
                return &(get_component(mData.at(index.mIndex)));
            }

            Entity *getEntity(const EntityComponentHandle<EntityComponentBase> &index) const override final
            {
                assert(index.mType == UniqueComponent::component_index<T>());
                if (index)
                    return getEntity(index.mIndex);
                else
                    return {};
            }

            Entity *getEntity(uint32_t index) const
            {
                return get_entity_ptr(mData.at(index));
            }

            EntityComponentOwningHandle<EntityComponentBase> emplace(const ObjectPtr &table, Entity *entity) override final
            {   
                typename Vector::iterator it = Engine::emplace(mData, mData.end(), table, entity);
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
                Serialize::set_synced(get_component(mData.at(index.mIndex)), synced);
            }

            void setActive(const EntityComponentHandle<EntityComponentBase> &index, bool active, bool existenceChanged) override final
            {
                Serialize::setActive(get_component(mData.at(index.mIndex)), active, existenceChanged);
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
                return get_component(mData.front());
            }

            T &operator[](size_t index)
            {
                return get_component(mData[index]);
            }

            Vector mData;
        };

    }
}
}