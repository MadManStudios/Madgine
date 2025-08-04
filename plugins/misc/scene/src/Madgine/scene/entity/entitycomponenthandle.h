#pragma once

#include "Generic/callerhierarchy.h"

#include "Meta/serialize/streams/streamresult.h"

#include "Modules/uniquecomponent/component_index.h"

#include "Meta/serialize/hierarchy/serializableunitptr.h"

namespace Engine {
namespace Scene {

    namespace Entity {

        void entityComponentHelperWrite(Serialize::FormattedSerializeStream &out, const EntityComponentHandle &index, const char *name, CallerHierarchyBasePtr hierarchy);
        Serialize::StreamResult entityComponentHelperRead(Serialize::FormattedSerializeStream &in, const EntityComponentHandle &index, const char *name, CallerHierarchyBasePtr hierarchy);
        Serialize::StreamResult entityComponentHelperApplyMap(Serialize::FormattedSerializeStream &in, EntityComponentHandle &index, bool success, CallerHierarchyBasePtr hierarchy);
        void entityComponentHelperSetSynced(EntityComponentHandle &index, bool synced, CallerHierarchyBasePtr hierarchy);
        void entityComponentHelperSetActive(EntityComponentHandle &index, bool active, bool existenceChanged, CallerHierarchyBasePtr hierarchy);


        struct MADGINE_SCENE_EXPORT EntityComponentHandle {

            EntityComponentHandle(uint32_t type, EntityComponentBase* component)
                : mType(type)
                , mComponent(component)
            {

            }

            const uint32_t mType;
            EntityComponentBase *mComponent;

            std::string_view name() const;
            ScopePtr getTyped() const;

            auto operator<=>(const EntityComponentHandle &other) const
            {
                return mType <=> other.mType;
            }

            auto operator<=>(uint32_t type) const
            {
                return mType <=> type;
            }

            friend Serialize::StreamResult tag_invoke(const Serialize::apply_map_t &, EntityComponentHandle &handle, Serialize::FormattedSerializeStream &in, bool success, CallerHierarchyBasePtr hierarchy)
            {
                return entityComponentHelperApplyMap(in, handle, success, hierarchy);
            }

            friend void tag_invoke(const Serialize::set_synced_t &, EntityComponentHandle &handle, bool synced, CallerHierarchyBasePtr hierarchy)
            {
                entityComponentHelperSetSynced(handle, synced, hierarchy);
            }
        };

    }

}

namespace Serialize {

    template <typename... Configs>
    struct Operations<Scene::Entity::EntityComponentHandle, Configs...> {

        static StreamResult read(Serialize::FormattedSerializeStream &in, Scene::Entity::EntityComponentHandle &handle, const char *name, CallerHierarchyBasePtr hierarchy)
        {
            return entityComponentHelperRead(in, handle, name, hierarchy);
        }

        static void write(Serialize::FormattedSerializeStream &out, const Scene::Entity::EntityComponentHandle &handle, const char *name, CallerHierarchyBasePtr hierarchy)
        {
            entityComponentHelperWrite(out, handle, name, hierarchy);
        }

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
        {
            throw 0;
            //return SerializableDataPtr::visitStream<T>(in, name, visitor);
        }
    };

}

}