#include "../../scenelib.h"

#include "entity.h"

#include "Generic/execution/algorithm.h"

#include "Meta/serialize/helper/typedobjectserialize.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../scenemanager.h"
#include "entitycomponentbase.h"
#include "entitycomponentcollector.h"
#include "entitycomponentlistbase.h"

namespace Engine {

constexpr auto componentBuilder()
{
    std::array<Accessor, 32> accessors;

    return accessors;
}

void componentInit(std::array<Accessor, 32> &accessors)
{
#if ENABLE_PLUGINS
    Scene::Entity::EntityComponentCollector::addInitializer([&]() {
#endif
        size_t i = 0;
        for (const auto &[name, index] : Scene::Entity::EntityComponentRegistry::sComponentsByName()) {
            accessors[i] = { name.data(),
                [](const Accessor *self, const ScopePtr &entity) {
                    return scope_cast<Scene::Entity::Entity>(entity)->hasComponent(self->mName);
                },
                [](const Accessor *self, ValueType &ret, const ScopePtr &entity) {
                    uint32_t index = Scene::Entity::EntityComponentRegistry::sComponentsByName().at(self->mName);
                    to_ValueType(ret, ScopePtr { scope_cast<Scene::Entity::Entity>(entity)->getComponent(index), *Scene::Entity::EntityComponentRegistry::get(index).mType });
                },
                nullptr,
                ExtendedValueTypeDesc {
                    ExtendedValueTypeIndex { ValueTypeEnum::ScopeValue }, Scene::Entity::EntityComponentRegistry::get(index).mType } };
            i++;
        }
#if ENABLE_PLUGINS
    });
#endif
}

}

METATABLE_BEGIN(Engine::Scene::Entity::Entity)
    NAMED_MEMBER(Name, mName)
    READONLY_PROPERTY(Behaviors, behaviors)
    READONLY_PROPERTY(Lifetime, lifetimeBase)
METATABLE_DYNAMIC_END(componentBuilder, componentInit, Engine::Scene::Entity::Entity)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::Entity)
    FIELD(mComponents, Serialize::ParentCreator<&Engine::Scene::Entity::Entity::readComponent, &Engine::Scene::Entity::Entity::writeComponent, &Engine::Scene::Entity::Entity::clearComponents>)
    FIELD(mBehaviors)
SERIALIZETABLE_END(Engine::Scene::Entity::Entity)

namespace Engine {

namespace Scene {
    namespace Entity {
        /*Entity::Entity(const Entity &other, bool local)
            : SerializableUnit(other)
            , mName(other.mName)
            , mLocal(local)
            , mSceneManager(other.mSceneManager)
        {
        }*/

        /* Entity::Entity(Entity &&other, bool local)
            : SyncableUnitEx(std::move(other))
            , mName(std::move(other.mName))
            , mLocal(local)
            , mComponents(std::move(other.mComponents))
            , mSceneManager(other.mSceneManager)
        {
        }*/

        Entity::Entity(EntityHandle &handle, SceneContainer &container, const std::string &name)
            : mName(name)
            , mHandle(handle)
            , mContainer(container)
            , mLifetime(&container.lifetime())
        {
        }

        Entity::~Entity()
        {
            clearComponents();
        }

        const std::string &Entity::key() const
        {
            return mName;
        }

        const std::string &Entity::name() const
        {
            return mName;
        }

        EntityComponentBase *Entity::getComponent(uint32_t i)
        {
            auto it = mComponents.physical().find(i);
            if (it == mComponents.physical().end())
                return nullptr;
            return &it->mComponent;
        }

        const EntityComponentBase *Entity::getComponent(uint32_t i) const
        {
            auto it = mComponents.physical().find(i);
            if (it == mComponents.physical().end())
                return nullptr;
            return &it->mComponent;
        }

        EntityComponentBase *Entity::getComponent(std::string_view name)
        {
            return getComponent(EntityComponentRegistry::sComponentsByName().at(name));
        }

        const EntityComponentBase *Entity::getComponent(std::string_view name) const
        {
            return getComponent(EntityComponentRegistry::sComponentsByName().at(name));
        }

        bool Entity::hasComponent(size_t i)
        {
            return mComponents.contains(i);
        }

        bool Entity::hasComponent(std::string_view name)
        {
            return hasComponent(EntityComponentRegistry::sComponentsByName().at(name));
        }

        EntityComponentBase *Entity::addComponent(std::string_view name)
        {
            return addComponent(EntityComponentRegistry::sComponentsByName().at(name));
        }

        EntityComponentBase *Entity::addComponent(size_t i)
        {
            auto it = mComponents.physical().find(i);
            if (it != mComponents.physical().end()) {
                return &it->mComponent;
            } else {
                auto it = mComponents.emplace(i, sceneMgr().entityComponentList(i).emplace(*this));
                return &it->mComponent;
            }
        }

        void Entity::removeComponent(std::string_view name)
        {
            removeComponent(EntityComponentRegistry::sComponentsByName().at(name));
        }

        void Entity::removeComponent(size_t i)
        {
            auto it = mComponents.find(i);
            assert(it != mComponents.physical().end());
            EntityComponentBase &comp = it->mComponent;
            mComponents.erase(it);
            sceneMgr().entityComponentList(i).erase(comp);
        }

        void Entity::clearComponents()
        {
            while (!mComponents.empty())
                removeComponent(mComponents.begin()->mType);
        }

        void Entity::startLifetime()
        {
            throw 0;
        }

        void Entity::endLifetime()
        {
            mLifetime.end();
        }

        EntityPtr Entity::pointer()
        {
            return mSelf;
        }

        EntityHandle &Entity::handle()
        {
            return mHandle;
        }

        Debug::DebuggableLifetime<Behavior::get_named_d> &Entity::lifetime()
        {
            return mLifetime;
        }

        Debug::DebuggableLifetimeBase &Entity::lifetimeBase()
        {
            return mLifetime;
        }

        Serialize::StreamResult Entity::readComponent(Serialize::CallerHierarchyFormattedSerializeStream in, uint32_t &type, OutRef<EntityComponentBase> &ptr)
        {
            std::string name;
            STREAM_PROPAGATE_ERROR(Serialize::beginExtendedTypedRead(in, name));
            type = EntityComponentRegistry::sComponentsByName().at(name);
            ptr = sceneMgr().entityComponentList(type).emplace(*this);
            return {};
        }

        const char *Entity::writeComponent(Serialize::CallerHierarchyFormattedSerializeStream out, const EntityComponentHandle &p) const
        {
            return Serialize::beginExtendedTypedWrite(out, EntityComponentRegistry::sComponentName(p.mType));
        }

        void Entity::handleEntityEvent(const typename mutable_set<EntityComponentHandle, std::less<>>::iterator &it, int op)
        {
            switch (op) {
            case BEFORE | RESET:
                throw "TODO";
            case AFTER | RESET:
                throw "TODO";
            case AFTER | EMPLACE:
                sceneMgr().entityComponentList(it->mType).init(it->mComponent);
                break;
            case BEFORE | ERASE:
                sceneMgr().entityComponentList(it->mType).finalize(it->mComponent);
                break;
            }
        }

        SceneManager &Entity::sceneMgr() const
        {
            return mContainer.sceneMgr();
        }

        /* EntityComponentPtr<EntityComponentBase> Entity::Helper::operator()(const EntityComponentOwningHandle<EntityComponentBase> &p) const
        {
            return { p, &mEntity->sceneMgr() };
        }*/

        Behavior::BehaviorList &Entity::behaviors()
        {
            return mBehaviors;
        }

        SceneContainer &Entity::container()
        {
            return mContainer;
        }

        const SceneContainer &Entity::container() const
        {
            return mContainer;
        }

        void Entity::onActivate(Serialize::CallbackTiming timing, bool active, bool existenceChanged)
        {
            assert(existenceChanged);
            if (timing == Serialize::CallbackTiming::AFTER && active) {
                mBehaviors.instantiate(mLifetime);
            }
        }

        void Entity::dtor()
        {
            if (isSynced()) {
                Serialize::set_synced(*this, false);
            }

            container().remove(pointer());
        }

    }
}
}
