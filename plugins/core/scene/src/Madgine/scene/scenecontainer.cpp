#include "../scenelib.h"

#include "scenecontainer.h"

#include "scenemanager.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Generic/projections.h"

#include "Meta/serialize/container/noparent.h"

METATABLE_BEGIN(Engine::Scene::SceneContainer)
//TODO
//SYNCABLEUNIT_MEMBERS()
//READONLY_PROPERTY(entities, entities)
METATABLE_END(Engine::Scene::SceneContainer)

using Helper = Engine::Serialize::NoParent<Engine::Scene::SceneContainer>;
METATABLE_BEGIN_BASE(Helper, Engine::Scene::SceneContainer)
METATABLE_END(Helper)

static Engine::Threading::DataMutex::Lock static_lock(Engine::Scene::SceneContainer *container)
{
    return container->mutex().lock(Engine::AccessMode::WRITE);
}

SERIALIZETABLE_BEGIN(Engine::Scene::SceneContainer,
    Engine::Serialize::CallableGuard<&static_lock>)
FIELD(mEntities,
    Serialize::ParentCreator<&Engine::Scene::SceneContainer::readEntity, &Engine::Scene::SceneContainer::writeEntity>,
    Serialize::RequestPolicy::no_requests)
SERIALIZETABLE_END(Engine::Scene::SceneContainer)

namespace Engine {
namespace Scene {

    SceneContainer::SceneContainer(SceneManager &sceneMgr)
        : mManager(sceneMgr)        
        , mLifetime(&sceneMgr.lifetime())
    {
        startLifetime();
    }

    Entity::EntityPtr SceneContainer::findEntity(const std::string &name)
    {
        auto it = std::ranges::find(mEntities, name, projectionKey);
        if (it == mEntities.end()) {
            return {};
        }
        return &*it;
    }

    std::string SceneContainer::generateUniqueName()
    {
        static size_t itemCount = 0;
        return "Madgine_AutoGen_Name_"s + std::to_string(++itemCount);
    }

    Threading::DataMutex &SceneContainer::mutex()
    {
        return mManager.mutex();
    }

    SceneManager &SceneContainer::sceneMgr() const
    {
        return mManager;
    }

    void SceneContainer::remove(Entity::Entity *e)
    {
        auto it = std::ranges::find(mEntities, e, projectionAddressOf);
        mEntities.erase(it);
    }

    Serialize::StreamResult SceneContainer::readEntity(Serialize::FormattedSerializeStream &in, OutRef<SceneContainer> &mgr, std::string &name)
    {
        STREAM_PROPAGATE_ERROR(in.beginExtendedRead("Entity", 1));
        mgr = *this;
        return read(in, name, "name");
    }

    std::tuple<SceneContainer &, std::string> SceneContainer::createEntityData(const std::string &name)
    {
        std::string actualName = name.empty() ? generateUniqueName() : name;

        return make_tuple(std::ref(*this), actualName);
    }

    const char *SceneContainer::writeEntity(Serialize::FormattedSerializeStream &out, const Entity::Entity &entity) const
    {
        out.beginExtendedWrite("Entity", 1);
        write(out, entity.name(), "name");
        return "Entity";
    }

    Entity::EntityPtr SceneContainer::createEntity(const std::string &name,
        const std::function<void(Entity::Entity &)> &init)
    {
        auto toPtr = [this](const typename RefcountedContainer<std::deque<Entity::Entity>>::iterator &it) {            
            return Entity::EntityPtr { &*it};
        };
        if (init)
            return toPtr(TupleUnpacker::invokeFlatten(LIFT(mEntities.emplace_init, this), mEntities.end(), init, createEntityData(name)));
        else
            return toPtr(TupleUnpacker::invokeFlatten(emplace, mEntities, mEntities.end(), createEntityData(name)));
    }

    void SceneContainer::createEntityAsyncImpl(Serialize::GenericMessageReceiver receiver, const std::string &name, std::function<void(Entity::Entity &)> init)
    {
        Execution::detach(mutex().locked(AccessMode::WRITE, [this, name, init { std::move(init) }, receiver { std::move(receiver) }]() mutable {
            auto toPtr = [](const typename RefcountedContainer<std::deque<Entity::Entity>>::iterator &it) { return Entity::EntityPtr { &*it }; };
            if (init)
                Execution::detach(
                    TupleUnpacker::invokeFlatten(LIFT(mEntities.emplace_init_async, this), mEntities.end(), init, createEntityData(name))
                    | Execution::then(std::move(toPtr))
                    | Execution::then_receiver(std::move(receiver)));
            else
                Execution::detach(
                    TupleUnpacker::invokeFlatten(LIFT(mEntities.emplace_async, this), mEntities.end(), createEntityData(name))
                    | Execution::then(std::move(toPtr))
                    | Execution::then_receiver(std::move(receiver)));
        }));
    }

    void SceneContainer::startLifetime()
    {
        mManager.mLifetime.attach(mLifetime);
    }

    void SceneContainer::endLifetime()
    {
        mLifetime.end();
    }

    Debug::DebuggableLifetime<get_binding_d> &SceneContainer::lifetime()
    {
        return mLifetime;
    }

    Execution::SignalStub<const RefcountedContainer<std::deque<Entity::Entity>>::iterator &, int> &SceneContainer::entitiesSignal()
    {
        return mEntities.observer().signal();
    }

}
}
