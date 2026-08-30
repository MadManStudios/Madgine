#include "../scenelib.h"
#include "Madgine/serialize/memory/memorylib.h"

#include "scenecontainer.h"

#include "Generic/execution/execution.h"
#include "Generic/execution/sender.h"
#include "Generic/projections.h"

#include "Meta/serialize/container/noparent.h"
#include "Meta/serialize/formats.h"

#include "Madgine/serialize/memory/memorymanager.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "scenemanager.h"

METATABLE_BEGIN(Engine::Scene::SceneContainer)
    FUNCTION(createEntity, name, descriptor, parent, onSuccess, onError)
    // TODO
    // SYNCABLEUNIT_MEMBERS()
    READONLY_PROPERTY(entities, entities)
METATABLE_END(Engine::Scene::SceneContainer)

static Engine::Threading::DataMutex::Lock static_lock(Engine::Serialize::ContextPtr context)
{
    return context_get<Engine::Scene::SceneContainer>(context)->mutex().lock(Engine::AccessMode::WRITE);
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
        : mLifetime(&sceneMgr.lifetime())
        , mManager(sceneMgr)
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

    Serialize::StreamResult SceneContainer::readEntity(Serialize::FormattedSerializeStream &in, OutRef<SceneContainer> &mgr, std::string &name)
    {
        mgr = *this;
        STREAM_PROPAGATE_ERROR(in.beginExtendedRead("Entity", 1));
        return Serialize::read(in, name, "name");
    }

    std::tuple<SceneContainer &, std::string, Entity::EntityDescriptor, Entity::EntityPtr> SceneContainer::createEntityData(const std::string &name, Entity::EntityDescriptor init, Entity::EntityPtr parent)
    {
        std::string actualName = name.empty() ? generateUniqueName() : name;

        return make_tuple(std::ref(*this), actualName, std::move(init), parent);
    }

    const char *SceneContainer::writeEntity(Serialize::FormattedSerializeStream &out, const Entity::EntityHandle &handle) const
    {
        std::string name;
        Execution::access_binding(handle.ptr(), [&](Entity::Entity &entity) {
            name = entity.name();
        });
        if (name.empty()) {
            return nullptr;
        }
        out.beginExtendedWrite("Entity", 1);
        write(out, name, "name");

        return "Entity";
    }

    Execution::Future<Serialize::MessageResult, Entity::EntityPtr> SceneContainer::createEntity(const std::string &name, Entity::EntityDescriptor init, Entity::EntityPtr parent, Closure<void(Entity::EntityPtr)> cb, Closure<void(Serialize::MessageResult)> onError)
    {
        Execution::Promise<Serialize::MessageResult, Entity::EntityPtr> promise;
        Execution::Future<Serialize::MessageResult, Entity::EntityPtr> future = promise.getFuture();

        mLifetime.attach(
            mutex().locked(AccessMode::WRITE, [this, name, init { std::move(init) }, parent {std::move(parent)}]() mutable {
                return TupleUnpacker::invokeFlatten(LIFT(mEntities.emplace_async, this), mEntities.end(), createEntityData(name, std::move(init), std::move(parent)));
            })
            | Execution::let_value(std::identity {}) | Execution::then([cb { std::move(cb) }](auto it) { if (cb) cb(it->ptr()); return it->ptr(); }) | Execution::onError(onError ? std::move(onError) : [](Serialize::MessageResult) {}) | Execution::with_receiver(std::move(promise)));

        return future;
    }

    void SceneContainer::startLifetime()
    {
        mManager.mLifetime.attach(mLifetime | Behavior::context_set(this));
    }

    void SceneContainer::endLifetime()
    {
        mLifetime.end();
    }

    void SceneContainer::copy(const SceneContainer &other)
    {
        Serialize::MemoryManager mgr { "SceneCopy" };

        Memory::WritableByteBuffer buffer;

        Serialize::FormattedSerializeStream out = mgr.openWrite(buffer, Serialize::Formats::xml);

        Serialize::write(out, other, "Scene");

        Memory::ByteBuffer readBuffer { buffer.mData, buffer.mSize };

        Serialize::FormattedSerializeStream in = mgr.openRead(std::move(readBuffer), Serialize::Formats::xml);

        Serialize::readState(in, *this, "Scene");
    }

    Debug::DebuggableLifetime<Reflect::get_reflect_contextual> &SceneContainer::lifetime()
    {
        return mLifetime;
    }

    Execution::SignalStub<void, const SceneContainer::EntityContainer::iterator &, int> &SceneContainer::entitiesSignal()
    {
        return mEntities.observer().signal();
    }

    std::string_view SceneContainer::name() const
    {
        return mManager.containerName(this);
    }

}
}
