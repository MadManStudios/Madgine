#pragma once

#include "entitycomponenthandle.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        template <typename T>
        struct EntityComponentPtrBase;

        MADGINE_SCENE_EXPORT EntityComponentBase *resolveEntityComponentHandle(SceneManager *sceneMgr, size_t index, EntityComponentHandle<EntityComponentBase> handle);

        template <>
        struct MADGINE_SCENE_EXPORT EntityComponentPtrBase<EntityComponentBase> {
            EntityComponentPtrBase();

            EntityComponentPtrBase(EntityComponentHandle<EntityComponentBase> data, SceneManager *sceneMgr);
            EntityComponentPtrBase(const EntityComponentPtrBase<EntityComponentBase> &other);

            EntityComponentPtrBase(EntityComponentPtrBase<EntityComponentBase> &&other);

            ~EntityComponentPtrBase();

            const EntityComponentHandle<EntityComponentBase> &handle() const;

            explicit operator EntityComponentHandle<EntityComponentBase>() const;
            SceneManager *sceneMgr() const;

            uint32_t type() const;

            EntityComponentBase *get() const;
            Serialize::SerializableDataPtr getSerialized() const;

            EntityComponentBase *operator->() const
            {
                return get();
            }

            operator EntityComponentBase *() const;

            explicit operator bool() const;

            std::string_view name() const;

            ScopePtr getTyped() const;

        protected:
            EntityComponentHandle<EntityComponentBase> mHandle;
            SceneManager *mSceneMgr;
        };

        template <>
        struct MADGINE_SCENE_EXPORT EntityComponentPtrBase<const EntityComponentBase> {
            EntityComponentPtrBase();

            EntityComponentPtrBase(EntityComponentHandle<const EntityComponentBase> data, SceneManager *sceneMgr);

            EntityComponentPtrBase(const EntityComponentPtrBase<const EntityComponentBase> &other);

            EntityComponentPtrBase(const EntityComponentPtrBase<EntityComponentBase> &other);

            ~EntityComponentPtrBase();

            const EntityComponentHandle<const EntityComponentBase> &handle() const
            {
                return mHandle;
            }

            explicit operator EntityComponentHandle<const EntityComponentBase>() const;

            SceneManager *sceneMgr() const
            {
                return mSceneMgr;
            }

            size_t type() const
            {
                return mHandle.mType;
            }

        protected:
            EntityComponentHandle<const EntityComponentBase> mHandle;
            SceneManager *mSceneMgr;
        };

        template <typename T>
        struct EntityComponentPtrBase {
            EntityComponentPtrBase() = default;

            EntityComponentPtrBase(const EntityComponentPtrBase<T> &other)
                : mHandle { other.mHandle }
                , mSceneMgr(other.mSceneMgr)
            {
            }

            explicit EntityComponentPtrBase(const EntityComponentPtrBase<EntityComponentBase> &other)
                : mHandle { other.handle() }
                , mSceneMgr(other.sceneMgr())
            {
                assert(!other || UniqueComponent::component_index<T>() == other.type());
            }

            template <typename U>
            requires std::convertible_to<T *, U *>
                explicit EntityComponentPtrBase(const EntityComponentPtrBase<U> &other)
                : mHandle { other.handle() }
                , mSceneMgr(other.sceneMgr())
            {
            }

            EntityComponentPtrBase(EntityComponentHandle<T> data, SceneManager *sceneMgr)
                : mHandle(data)
                , mSceneMgr(sceneMgr)
            {
            }

            ~EntityComponentPtrBase()
            {
            }

            EntityComponentPtrBase<T> &operator=(const EntityComponentPtrBase<T> &other)
            {
                mHandle = other.mHandle;
                mSceneMgr = other.mSceneMgr;
                return *this;
            }

            EntityComponentPtrBase<T> &operator=(EntityComponentPtrBase<T> &&other)
            {
                mHandle = std::move(other.mHandle);
                std::swap(mSceneMgr, other.mSceneMgr);
                return *this;
            }

            const EntityComponentHandle<T> &handle() const
            {
                return mHandle;
            }

            explicit operator EntityComponentHandle<T>() const
            {
                return mHandle;
            }

            bool operator!=(const EntityComponentHandle<T> &handle) const
            {
                return mHandle != handle;
            }

            bool operator==(const EntityComponentHandle<T> &handle) const
            {
                return mHandle == handle;
            }

            SceneManager *sceneMgr() const
            {
                return mSceneMgr;
            }

            T *get() const
            {
                return mHandle ? static_cast<T *>(resolveEntityComponentHandle(mSceneMgr, UniqueComponent::component_index<T>(), mHandle)) : nullptr;
            }

            T *operator->() const
            {
                return get();
            }

            operator T *() const
            {
                return get();
            }

            void relocate(const EntityComponentHandle<EntityComponentBase> &index)
            {
                mHandle.relocate(index);
            }

        protected:
            EntityComponentHandle<T> mHandle;
            SceneManager *mSceneMgr;
        };

        template <typename T>
        struct EntityComponentPtr : EntityComponentPtrBase<T> {
            using EntityComponentPtrBase<T>::EntityComponentPtrBase;
        };

    }
}
}