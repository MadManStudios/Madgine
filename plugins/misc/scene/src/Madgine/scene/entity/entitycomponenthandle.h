#pragma once

#include "Generic/callerhierarchy.h"

#include "Meta/serialize/streams/streamresult.h"

#include "Modules/uniquecomponent/component_index.h"

#include "Meta/serialize/hierarchy/serializableunitptr.h"

namespace Engine {
namespace Scene {

    namespace Entity {

        void entityComponentHelperWrite(Serialize::CallerHierarchyFormattedSerializeStream out, const EntityComponentHandle &index, const char *name);
        Serialize::StreamResult entityComponentHelperRead(Serialize::CallerHierarchyFormattedSerializeStream in, const EntityComponentHandle &index, const char *name);
        Serialize::StreamResult entityComponentHelperApplyMap(Serialize::CallerHierarchyFormattedSerializeStream in, EntityComponentHandle &index, bool success);
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

            friend Serialize::StreamResult tag_invoke(const Serialize::apply_map_t &, EntityComponentHandle &handle, Serialize::CallerHierarchyFormattedSerializeStream in, bool success)
            {
                return entityComponentHelperApplyMap(in, handle, success);
            }

            friend void tag_invoke(const Serialize::set_synced_t &, EntityComponentHandle &handle, bool synced, CallerHierarchyBasePtr hierarchy)
            {
                entityComponentHelperSetSynced(handle, synced, hierarchy);
            }

            template <typename... Configs>
            friend void tag_invoke(Serialize::set_active_t<Configs...>, EntityComponentHandle &handle, bool active, bool existenceChanged, const CallerHierarchyBasePtr &hierarchy)
            {
                entityComponentHelperSetActive(handle, active, existenceChanged, hierarchy);
            }
        };

    }

}

namespace Serialize {

    template <typename... Configs>
    struct Operations<Scene::Entity::EntityComponentHandle, Configs...> {

        static StreamResult read(Serialize::CallerHierarchyFormattedSerializeStream in, Scene::Entity::EntityComponentHandle &handle, const char *name)
        {
            return entityComponentHelperRead(in, handle, name);
        }

        static void write(Serialize::CallerHierarchyFormattedSerializeStream out, const Scene::Entity::EntityComponentHandle &handle, const char *name)
        {
            entityComponentHelperWrite(out, handle, name);
        }

        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor)
        {
            throw 0;
            //return SerializableDataPtr::visitStream<T>(in, name, visitor);
        }
    };

}

}