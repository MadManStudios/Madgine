#pragma once

#include "Generic/callerhierarchy.h"

#include "Meta/serialize/streams/streamresult.h"

#include "Modules/uniquecomponent/component_index.h"

#include "Meta/serialize/hierarchy/serializableunitptr.h"

namespace Engine {
namespace Scene {

    namespace Entity {

        template <typename T>
        struct EntityComponentHandle;

        void entityComponentHelperWrite(Serialize::FormattedSerializeStream &out, const EntityComponentHandle<EntityComponentBase> &index, const char *name, const SceneManager *mgr);
        Serialize::StreamResult entityComponentHelperRead(Serialize::FormattedSerializeStream &in, EntityComponentHandle<EntityComponentBase> &index, const char *name, SceneManager *mgr);

        void entityComponentOwningHelperWrite(Serialize::FormattedSerializeStream &out, const EntityComponentHandle<EntityComponentBase> &index, const char *name, CallerHierarchyBasePtr hierarchy);
        Serialize::StreamResult entityComponentOwningHelperRead(Serialize::FormattedSerializeStream &in, const EntityComponentHandle<EntityComponentBase> &index, const char *name, CallerHierarchyBasePtr hierarchy);
        Serialize::StreamResult entityComponentOwningHelperApplyMap(Serialize::FormattedSerializeStream &in, EntityComponentHandle<EntityComponentBase> &index, bool success, CallerHierarchyBasePtr hierarchy);
        void entityComponentOwningHelperSetSynced(EntityComponentHandle<EntityComponentBase> &index, bool synced, CallerHierarchyBasePtr hierarchy);
        void entityComponentOwningHelperSetActive(EntityComponentHandle<EntityComponentBase> &index, bool active, bool existenceChanged, CallerHierarchyBasePtr hierarchy);

        template <>
        struct EntityComponentHandle<EntityComponentBase> {
            IndexType<uint32_t> mIndex;
            IndexType<uint32_t> mType;

            bool operator!=(const EntityComponentHandle<EntityComponentBase> &other) const
            {
                assert(mType == other.mType);
                return mIndex != other.mIndex;
            }

            bool operator==(const EntityComponentHandle<EntityComponentBase> &other) const
            {
                assert(mType == other.mType);
                return mIndex == other.mIndex;
            }

            explicit operator bool() const
            {
                return bool(mIndex);
            }

            uint32_t type() const
            {
                return mType;
            }

            void relocate(const EntityComponentHandle<EntityComponentBase> &index)
            {
                assert(mType == index.mType);
                mIndex = index.mIndex;
            }
        };

        template <>
        struct EntityComponentHandle<const EntityComponentBase> {
            IndexType<uint32_t> mIndex;
            IndexType<uint32_t> mType;

            EntityComponentHandle() = default;

            EntityComponentHandle(EntityComponentHandle<EntityComponentBase> handle)
                : mIndex(handle.mIndex)
                , mType(handle.mType)
            {
            }

            bool operator!=(const EntityComponentHandle<EntityComponentBase> &other) const
            {
                assert(mType == other.mType);
                return mIndex != other.mIndex;
            }

            bool operator==(const EntityComponentHandle<EntityComponentBase> &other) const
            {
                assert(mType == other.mType);
                return mIndex == other.mIndex;
            }

            explicit operator bool() const
            {
                return bool(mIndex);
            }

            uint32_t type() const
            {
                return mType;
            }

            void relocate(const EntityComponentHandle<EntityComponentBase> &index)
            {
                assert(mType == index.mType);
                mIndex = index.mIndex;
            }
        };

        template <typename T>
        struct EntityComponentHandle {
            IndexType<uint32_t> mIndex;

            EntityComponentHandle() = default;

            EntityComponentHandle(IndexType<uint32_t> index)
                : mIndex(index)
            {
            }

            EntityComponentHandle(const EntityComponentHandle<std::conditional_t<std::is_const_v<T>, const EntityComponentBase, EntityComponentBase>> &other)
                : mIndex(other.mIndex)
            {
                assert(!other || UniqueComponent::component_index<T>() == other.mType);
            }

            Serialize::StreamResult readState(Serialize::FormattedSerializeStream &in, const char *name, SceneManager *mgr)
            {
                EntityComponentHandle<EntityComponentBase> handle { IndexType<uint32_t>::sInvalid, type() };
                STREAM_PROPAGATE_ERROR(entityComponentHelperRead(in, handle, name, mgr));
                mIndex = handle.mIndex;
                return {};
            }

            void writeState(Serialize::FormattedSerializeStream &out, const char *name, const SceneManager *mgr) const
            {
                entityComponentHelperWrite(out, *this, name, mgr);
            }

            bool operator!=(const EntityComponentHandle<T> &other) const
            {
                return mIndex != other.mIndex;
            }

            bool operator==(const EntityComponentHandle<T> &other) const
            {
                return mIndex == other.mIndex;
            }

            explicit operator bool() const
            {
                return bool(mIndex);
            }

            uint32_t type() const
            {
                return UniqueComponent::component_index<T>();
            }

            operator EntityComponentHandle<EntityComponentBase>() const
            {
                return { mIndex, type() };
            }

            void relocate(const EntityComponentHandle<EntityComponentBase> &index)
            {
                assert(index.mType == type());
                mIndex = index.mIndex;
            }
        };

        template <typename T>
        struct EntityComponentOwningHandle {
            EntityComponentHandle<T> mHandle;

            EntityComponentOwningHandle() = default;
            EntityComponentOwningHandle(EntityComponentHandle<T> handle)
                : mHandle(std::move(handle))
            {
            }
            EntityComponentOwningHandle(const EntityComponentOwningHandle<T> &other) = delete;
            EntityComponentOwningHandle(EntityComponentOwningHandle<T> &&other) = default;

            EntityComponentOwningHandle &operator=(const EntityComponentOwningHandle &) = delete;
            EntityComponentOwningHandle &operator=(EntityComponentOwningHandle<T> &&) = default;

            operator EntityComponentHandle<T>() const
            {
                return mHandle;
            }

            auto operator<=>(const EntityComponentOwningHandle<T> &other) const
            {
                return mHandle.type() <=> other.mHandle.type();
            }

            auto operator<=>(uint32_t type) const
            {
                return mHandle.type() <=> type;
            }

            void relocate(const EntityComponentHandle<EntityComponentBase> &index)
            {
                mHandle.relocate(index);
            }

            friend Serialize::StreamResult tag_invoke(const Serialize::apply_map_t&, Scene::Entity::EntityComponentOwningHandle<T> &handle, Serialize::FormattedSerializeStream &in, bool success, CallerHierarchyBasePtr hierarchy)
            {
                return entityComponentOwningHelperApplyMap(in, handle.mHandle, success, hierarchy);
            }

            friend void tag_invoke(const Serialize::set_synced_t &, Scene::Entity::EntityComponentOwningHandle<T> &handle, bool synced, CallerHierarchyBasePtr hierarchy)
            {
                entityComponentOwningHelperSetSynced(handle.mHandle, synced, hierarchy);
            }
        };

    }

}

namespace Serialize {

    template <typename T, typename... Configs>
    struct Operations<Scene::Entity::EntityComponentOwningHandle<T>, Configs...> {

        static StreamResult read(Serialize::FormattedSerializeStream &in, Scene::Entity::EntityComponentOwningHandle<T> &handle, const char *name, CallerHierarchyBasePtr hierarchy)
        {
            return entityComponentOwningHelperRead(in, handle.mHandle, name, hierarchy);
        }

        static void write(Serialize::FormattedSerializeStream &out, const Scene::Entity::EntityComponentOwningHandle<T> &handle, const char *name, CallerHierarchyBasePtr hierarchy)
        {
            entityComponentOwningHelperWrite(out, handle.mHandle, name, hierarchy);
        }

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
        {
            return SerializableDataPtr::visitStream<T>(in, name, visitor);
        }

        static void setActive(Scene::Entity::EntityComponentOwningHandle<T> &handle, bool active, bool existenceChanged, CallerHierarchyBasePtr hierarchy)
        {
            entityComponentOwningHelperSetActive(handle.mHandle, active, existenceChanged, hierarchy);
        }
    };

    template <typename T, typename... Configs>
    struct Operations<Scene::Entity::EntityComponentHandle<T>, Configs...> {

        static StreamResult read(Serialize::FormattedSerializeStream &in, Scene::Entity::EntityComponentOwningHandle<T> &handle, const char *name, Scene::SceneManager *mgr)
        {
            return entityComponentHelperRead(in, handle, name, mgr);
        }

        static void write(Serialize::FormattedSerializeStream &out, const Scene::Entity::EntityComponentOwningHandle<T> &handle, const char *name, Scene::SceneManager *mgr)
        {
            entityComponentHelperWrite(out, handle, name, mgr);
        }

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
        {
            throw 0;
            return {};
        }
    };

}

}