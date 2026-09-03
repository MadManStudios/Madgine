#pragma once

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT EntityComponentListBase {
            virtual ~EntityComponentListBase() = default;

            virtual Reflect::ScopePtr getTyped(EntityComponentBase &comp) = 0;
            virtual Serialize::SerializableDataPtr getSerialized(EntityComponentBase &comp) = 0;
            virtual Serialize::SerializableDataConstPtr getSerialized(const EntityComponentBase &comp) const = 0;
            virtual const Serialize::SerializeTable *serializeTable() const = 0;
            virtual void init(EntityComponentBase &comp, Entity &entity) = 0;
            virtual void finalize(EntityComponentBase &comp, Entity &entity) = 0;
            virtual EntityComponentBase &emplace(Entity &entity) = 0;
            virtual EntityComponentBase &emplace(Entity &entity, const EntityComponentBase &source) = 0;
            virtual void erase(EntityComponentBase &comp) = 0;
            virtual bool empty() = 0;
            virtual void clear() = 0;
            virtual size_t size() const = 0;

            virtual void setSynced(EntityComponentBase &comp, bool synced) = 0;
            virtual void setActive(EntityComponentBase &comp, bool active, bool existenceChanged) = 0;

            Serialize::StreamResult readState(EntityComponentBase &comp, Serialize::FormattedSerializeStream &in, const char *name, Serialize::ContextPtr context);

            void writeState(EntityComponentBase &comp, Serialize::FormattedSerializeStream &out, const char *name, Serialize::ContextPtr context) const;

            Serialize::StreamResult applyMap(EntityComponentBase &comp, Serialize::FormattedSerializeStream &in, bool success);
        };

    }
}
}