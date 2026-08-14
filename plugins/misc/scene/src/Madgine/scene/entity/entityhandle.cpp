#include "../../scenelib.h"

#include "entityhandle.h"

#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "Meta/serialize/serializetable_impl.h"

#include "../scenecontainer.h"
#include "../scenemanager.h"
#include "entitycomponentbase.h"
#include "entitycomponentcollector.h"
#include "entitycomponentlistbase.h"
#include "entitysender.h"

using namespace Engine::Serialize;
static constexpr Serializer sComponentSynchronizer {
    "ComponentSynchronizer",
    {},
    [](const void *, FormattedSerializeStream &, const char *, ContextPtr context) {
    },
    [](void *, FormattedSerializeStream &, const char *, ContextPtr context) -> StreamResult {
        throw 0;
        return {};
    },
    [](void *unit, FormattedSerializeStream &in, PendingRequest &request, ContextPtr context) -> StreamResult {
        Engine::Scene::Entity::EntityHandle *handle = unit_cast<Engine::Scene::Entity::EntityHandle>(unit);

        Engine::Serialize::StreamResult result;

        Engine::Execution::access_binding(handle->ptr(), [&](Engine::Scene::Entity::Entity &entity) {
            result = [&]() -> Engine::Serialize::StreamResult {
                std::string name;
                STREAM_PROPAGATE_ERROR(read(in, name, "name"));
                auto it = Engine::Scene::Entity::EntityComponentRegistry::sComponentsByName().find(name);
                if (it == Engine::Scene::Entity::EntityComponentRegistry::sComponentsByName().end())
                    return STREAM_INTEGRITY_ERROR(in) << "Received message for component '" << name << "', which is not registered.";

                Engine::Scene::Entity::EntityComponentBase *component = entity.getComponent(it->second);
                if (!component)
                    return STREAM_INTEGRITY_ERROR(in) << "Received message for component '" << name << "', which is not a component of this Entity.";
                SerializableDataPtr serializedComponent = entity.sceneMgr().entityComponentList(it->second).getSerialized(*component);
                return serializedComponent.mType->readAction(serializedComponent.unit(), in, request, context);
            }();
        });

        return result;
    },
    [](void *, FormattedMessageStream &, MessageId) -> StreamResult {
        throw 0;
        return {};
    },
    [](const Serializer *, void *unit, FormattedSerializeStream &in, bool success, ContextPtr context) -> StreamResult {
        Engine::Scene::Entity::EntityHandle *handle = unit_cast<Engine::Scene::Entity::EntityHandle>(unit);
        Engine::Serialize::StreamResult result;
        Engine::Execution::access_binding(handle->ptr(), [&](Engine::Scene::Entity::Entity &entity) {
            result = Engine::Serialize::apply_map(entity, in, success, context);
        });
        return result;
    },
    [](const Serializer *, void *unit, bool b, ContextPtr context) {
        Engine::Scene::Entity::EntityHandle *handle = unit_cast<Engine::Scene::Entity::EntityHandle>(unit);
        Engine::Execution::access_binding(handle->ptr(), [&](Engine::Scene::Entity::Entity &entity) {
            Engine::Serialize::set_synced(entity, b, context);
        });
    },
    [](const Serializer *, void *unit, bool active, bool existenceChanged, ContextPtr context) {
        Engine::Scene::Entity::EntityHandle *handle = unit_cast<Engine::Scene::Entity::EntityHandle>(unit);
        Engine::Execution::access_binding(handle->ptr(), [&](Engine::Scene::Entity::Entity &entity) {
            Engine::Serialize::set_active<>(entity, active, existenceChanged, context);
        });
    },
    [](const Serializer *, void *unit) {
        Engine::Scene::Entity::EntityHandle *handle = unit_cast<Engine::Scene::Entity::EntityHandle>(unit);
        Engine::Execution::access_binding(handle->ptr(), [&](Engine::Scene::Entity::Entity &entity) {
            Engine::Serialize::set_parent(entity, handle);
        });
    },
    [](const void *unit, const std::vector<WriteMessage> &outStreams, void *data) {
        const Engine::Scene::Entity::EntityHandle *handle = unit_cast<Engine::Scene::Entity::EntityHandle>(unit);

        Engine::Execution::access_binding(handle->ptr(), [&](Engine::Scene::Entity::Entity &entity) {
            Engine::Scene::Entity::EntityComponentActionPayload &payload = *static_cast<Engine::Scene::Entity::EntityComponentActionPayload *>(data);
            for (FormattedMessageStream &stream : outStreams) {
                write(stream, Engine::Scene::Entity::EntityComponentRegistry::sComponentName(payload.mComponentIndex), "name");
            }

            const SerializeTable *type = entity.sceneMgr().entityComponentList(payload.mComponentIndex).serializeTable();
            uint16_t index = type->getIndex(payload.mOffset);
            type->writeAction(payload.mComponent, index, outStreams, payload.mData);
        });
    },
    [](const void *, FormattedSerializeStream &out, void *, ContextPtr context) { throw 0; }
};

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::EntityHandle)
    SERIALIZETABLE_ENTRY(sComponentSynchronizer)
SERIALIZETABLE_END(Engine::Scene::Entity::EntityHandle)

namespace Engine {
namespace Scene {
    namespace Entity {

        EntityHandle::EntityHandle(SceneContainer &container, const std::string &name, const EntityDescriptor &init)
        {
            container.lifetime().attach(EntitySender { {}, *this, container, name, [&](Entity &e) {
                                                          init.apply(e);                                                              
                                                          mPtr = e.pointer();
                                                      } });
            assert(mPtr);
        }

        const EntityPtr &EntityHandle::ptr() const
        {
            return mPtr;
        }

    }
}
}
