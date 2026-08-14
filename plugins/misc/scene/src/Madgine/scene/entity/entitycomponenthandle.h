#pragma once

#include "Meta/serialize/hierarchy/serializableunitptr.h"
#include "Meta/serialize/streams/streamresult.h"

#include "Modules/uniquecomponent/component_index.h"

namespace Engine {
namespace Scene {

    namespace Entity {

        void entityComponentHelperWrite(Serialize::FormattedSerializeStream &out, const EntityComponentHandle &index, const char *name, Serialize::ContextPtr context);
        Serialize::StreamResult entityComponentHelperRead(Serialize::FormattedSerializeStream &in, const EntityComponentHandle &index, const char *name, Serialize::ContextPtr context);
        Serialize::StreamResult entityComponentHelperApplyMap(Serialize::FormattedSerializeStream &in, EntityComponentHandle &index, bool success, Serialize::ContextPtr context);
        void entityComponentHelperSetSynced(EntityComponentHandle &index, bool synced, Serialize::ContextPtr context);
        void entityComponentHelperSetActive(EntityComponentHandle &index, bool active, bool existenceChanged, Serialize::ContextPtr context);

        struct MADGINE_SCENE_EXPORT EntityComponentHandle {

            EntityComponentHandle(uint32_t type, EntityComponentBase &component)
                : mType(type)
                , mComponent(component)
            {
            }

            const uint32_t mType;
            EntityComponentBase &mComponent;

            std::string_view name() const;
            Reflect::ScopePtr getTyped() const;

            auto operator<=>(const EntityComponentHandle &other) const
            {
                return mType <=> other.mType;
            }

            auto operator<=>(uint32_t type) const
            {
                return mType <=> type;
            }

            template <typename Context>
            friend Serialize::StreamResult tag_invoke(const Serialize::apply_map_t &, EntityComponentHandle &handle, Serialize::FormattedSerializeStream &in, bool success, Context &&context)
            {
                return entityComponentHelperApplyMap(in, handle, success, context);
            }

            template <typename Context>
            friend void tag_invoke(const Serialize::set_synced_t &, EntityComponentHandle &handle, bool synced, Context &&context)
            {
                entityComponentHelperSetSynced(handle, synced, context);
            }

            template <typename... Configs, typename Context>
            friend void tag_invoke(Serialize::set_active_t<Configs...>, EntityComponentHandle &handle, bool active, bool existenceChanged, Context &&context)
            {
                entityComponentHelperSetActive(handle, active, existenceChanged, context);
            }
        };

    }

}

namespace Serialize {

    template <typename... Configs>
    struct Operations<Scene::Entity::EntityComponentHandle, Configs...> {

        template <typename Context>
        static StreamResult read(Serialize::FormattedSerializeStream &in, Scene::Entity::EntityComponentHandle &handle, const char *name, Context &&context)
        {
            return entityComponentHelperRead(in, handle, name, context);
        }

        template <typename Context>
        static void write(Serialize::FormattedSerializeStream &out, const Scene::Entity::EntityComponentHandle &handle, const char *name, Context &&context)
        {
            entityComponentHelperWrite(out, handle, name, context);
        }

        template <typename Context>
        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
        {
            throw 0;
            // return SerializableDataPtr::visitStream<T>(in, name, visitor);
        }
    };

}

}