#pragma once

#include "entitycomponenthandle.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT EntityComponentListBase {
            virtual ~EntityComponentListBase() = default;

            virtual EntityComponentBase *get(const EntityComponentHandle<EntityComponentBase> &index) = 0;
            virtual const EntityComponentBase *get(const EntityComponentHandle<EntityComponentBase> &index) const = 0;
            virtual ScopePtr getTyped(const EntityComponentHandle<EntityComponentBase> &index) = 0;
            virtual Serialize::SerializableDataPtr getSerialized(const EntityComponentHandle<EntityComponentBase> &index) = 0;
            virtual Serialize::SerializableDataConstPtr getSerialized(const EntityComponentHandle<EntityComponentBase> &index) const = 0;
            virtual const Serialize::SerializeTable *serializeTable() const = 0;
            virtual void init(const EntityComponentHandle<EntityComponentBase> &index, Entity *entity) = 0;
            virtual void finalize(const EntityComponentHandle<EntityComponentBase> &index, Entity *entity) = 0;
            virtual Entity *getEntity(const EntityComponentHandle<EntityComponentBase> &index) const = 0; 
            virtual EntityComponentOwningHandle<EntityComponentBase> emplace(Entity *entity) = 0;
            virtual void erase(const EntityComponentHandle<EntityComponentBase> &index) = 0;
            virtual bool empty() = 0;
            virtual void clear() = 0;
            virtual size_t size() const = 0;

            virtual void setSynced(const EntityComponentHandle<EntityComponentBase> &index, bool synced) = 0;
            virtual void setActive(const EntityComponentHandle<EntityComponentBase> &index, bool active, bool existenceChanged) = 0;

            
            Serialize::StreamResult readState(const EntityComponentHandle<EntityComponentBase> &index, Serialize::FormattedSerializeStream &in, const char *name, CallerHierarchyBasePtr hierarchy);

            void writeState(const EntityComponentHandle<EntityComponentBase> &index, Serialize::FormattedSerializeStream &out, const char *name, CallerHierarchyBasePtr hierarchy) const;

            Serialize::StreamResult applyMap(const EntityComponentHandle<EntityComponentBase> &index, Serialize::FormattedSerializeStream &in, bool success, CallerHierarchyBasePtr hierarchy);

        };

    }
}
}

REGISTER_TYPE(Engine::Scene::Entity::EntityComponentListBase)