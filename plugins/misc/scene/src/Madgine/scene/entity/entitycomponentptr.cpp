#include "../../scenelib.h"

#include "entitycomponentptr.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "../scenemanager.h"

#include "entitycomponentlist.h"

#include "Generic/projections.h"

using EntityComponentBasePtr = Engine::Scene::Entity::EntityComponentPtr<Engine::Scene::Entity::EntityComponentBase>;

METATABLE_BEGIN(EntityComponentBasePtr);
PROXY(getTyped)
METATABLE_END(EntityComponentBasePtr);

namespace Engine {

namespace Scene {
    namespace Entity {

        EntityComponentBase *resolveEntityComponentHandle(SceneManager *sceneMgr, size_t index, EntityComponentHandle<EntityComponentBase> handle)
        {
            return sceneMgr->entityComponentList(index).get(handle);
        }

        EntityComponentPtrBase<EntityComponentBase>::EntityComponentPtrBase() = default;

        EntityComponentPtrBase<EntityComponentBase>::EntityComponentPtrBase(EntityComponentHandle<EntityComponentBase> data, SceneManager *sceneMgr)
            : mHandle(data)
            , mSceneMgr(sceneMgr)
        {
        }

        EntityComponentPtrBase<EntityComponentBase>::EntityComponentPtrBase(const EntityComponentPtrBase<EntityComponentBase> &other)
            : mHandle(other.mHandle)
            , mSceneMgr(other.mSceneMgr)
        {
        }

        EntityComponentPtrBase<EntityComponentBase>::EntityComponentPtrBase(EntityComponentPtrBase<EntityComponentBase> &&other)
            : mHandle(std::move(other.mHandle))
            , mSceneMgr(std::exchange(other.mSceneMgr, nullptr))
        {
        }

        EntityComponentPtrBase<EntityComponentBase>::~EntityComponentPtrBase()
        {
        }

        EntityComponentPtrBase<EntityComponentBase>::operator EntityComponentHandle<EntityComponentBase>() const
        {
            return mHandle;
        }

        EntityComponentBase *EntityComponentPtrBase<EntityComponentBase>::get() const
        {
            return mSceneMgr->entityComponentList(mHandle.mType).get(mHandle);
        }

        Serialize::SerializableDataPtr EntityComponentPtrBase<EntityComponentBase>::getSerialized() const
        {
            return mSceneMgr->entityComponentList(mHandle.mType).getSerialized(mHandle);
        }

        SceneManager *EntityComponentPtrBase<EntityComponentBase>::sceneMgr() const
        {
            return mSceneMgr;
        }

        uint32_t EntityComponentPtrBase<EntityComponentBase>::type() const
        {
            return mHandle.mType;
        }

        EntityComponentPtrBase<EntityComponentBase>::operator EntityComponentBase *() const
        {
            return get();
        }

        std::string_view EntityComponentPtrBase<EntityComponentBase>::name() const
        {
            return std::ranges::find(EntityComponentRegistry::sComponentsByName(), mHandle.mType, projectionPairSecond)->first;
        }

        ScopePtr EntityComponentPtrBase<EntityComponentBase>::getTyped() const
        {
            return mSceneMgr->entityComponentList(mHandle.mType).getTyped(mHandle);
        }

        const EntityComponentHandle<EntityComponentBase> &EntityComponentPtrBase<EntityComponentBase>::handle() const
        {
            return mHandle;
        }

        EntityComponentPtrBase<EntityComponentBase>::operator bool() const
        {
            return mHandle.operator bool();
        }

        EntityComponentPtrBase<const EntityComponentBase>::EntityComponentPtrBase() = default;

        EntityComponentPtrBase<const EntityComponentBase>::EntityComponentPtrBase(const EntityComponentPtrBase<const EntityComponentBase> &other)
            : mHandle(other.mHandle)
            , mSceneMgr(other.mSceneMgr)
        {
        }

        EntityComponentPtrBase<const EntityComponentBase>::EntityComponentPtrBase(const EntityComponentPtrBase<EntityComponentBase> &other)
            : mHandle { other.handle() }
            , mSceneMgr(other.sceneMgr())
        {
        }

        EntityComponentPtrBase<const EntityComponentBase>::EntityComponentPtrBase(EntityComponentHandle<const EntityComponentBase> data, SceneManager *sceneMgr)
            : mHandle(data)
            , mSceneMgr(sceneMgr)
        {
        }

        EntityComponentPtrBase<const EntityComponentBase>::~EntityComponentPtrBase()
        {
        }

        EntityComponentPtrBase<const EntityComponentBase>::operator EntityComponentHandle<const EntityComponentBase>() const
        {
            return mHandle;
        }

    }
}
}