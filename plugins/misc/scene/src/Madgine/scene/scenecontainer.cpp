#include "../scenelib.h"

#include "scenecontainer.h"

#include "scenemanager.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Generic/projections.h"

#include "Meta/serialize/container/noparent.h"

#include "Generic/execution/execution.h"

METATABLE_BEGIN(Engine::Scene::SceneContainer)
// TODO
// SYNCABLEUNIT_MEMBERS()
READONLY_PROPERTY(entities, entities)
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
        auto it = std::ranges::find_if(mEntities, [&](Entity::EntityHandle &handle) {
            return Execution::access_binding(handle.ptr(), [&](Entity::Entity &e) {
                return e.name() == name;
            });
        });
        if (it == mEntities.end()) {
            return {};
        }
        return it->ptr();
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

    void SceneContainer::remove(Entity::EntityPtr e)
    {
        auto it = std::ranges::find(mEntities, e, &Entity::EntityHandle::ptr);
        assert(it != mEntities.end());
        mEntities.erase(it);
    }

    Serialize::StreamResult SceneContainer::readEntity(Serialize::CallerHierarchyFormattedSerializeStream in, OutRef<SceneContainer> &mgr, std::string &name)
    {
        mgr = *this;
        STREAM_PROPAGATE_ERROR(in.mStream.beginExtendedRead("Entity", 1));
        return Serialize::read(in, name, "name");
    }

    std::tuple<SceneContainer &, std::string, std::function<void(Entity::Entity &)>> SceneContainer::createEntityData(const std::string &name, std::function<void(Entity::Entity &)> init)
    {
        std::string actualName = name.empty() ? generateUniqueName() : name;

        return make_tuple(std::ref(*this), actualName, init);
    }

    const char *SceneContainer::writeEntity(Serialize::CallerHierarchyFormattedSerializeStream out, const Entity::EntityHandle &handle) const
    {
        std::string name;
        Execution::access_binding(handle.ptr(), [&](Entity::Entity &entity) {
            name = entity.name();
        });
        if (name.empty()) {
            return nullptr;
        }
        out.mStream.beginExtendedWrite("Entity", 1);
        write(out, name, "name");

        return "Entity";
    }

    Entity::EntityPtr SceneContainer::createEntity(const std::string &name,
        const std::function<void(Entity::Entity &)> &init)
    {
        return TupleUnpacker::invokeFlatten(LIFT(mEntities.emplace, this), mEntities.end(), createEntityData(name, init))->ptr();
    }

    void SceneContainer::createEntityAsyncImpl(Serialize::GenericMessageReceiver receiver, const std::string &name, std::function<void(Entity::Entity &)> init)
    {
        Execution::detach(mutex().locked(AccessMode::WRITE, [this, name, init { std::move(init) }, receiver { std::move(receiver) }]() mutable {
            auto toPtr = [](const typename EntityContainer::iterator &it) { return it->ptr(); };

            Execution::detach_with_receiver(
                TupleUnpacker::invokeFlatten(LIFT(mEntities.emplace_async, this), mEntities.end(), createEntityData(name, std::move(init)))
                    | Execution::then(std::move(toPtr)),
                std::move(receiver));
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

    Debug::DebuggableLifetime<Behavior::get_named_d> &SceneContainer::lifetime()
    {
        return mLifetime;
    }

    Execution::SignalStub<const SceneContainer::EntityContainer::iterator &, int> &SceneContainer::entitiesSignal()
    {
        return mEntities.observer().signal();
    }

}
}
