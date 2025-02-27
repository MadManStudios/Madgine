#include "../../scenelib.h"
#include "entitycomponenthandle.h"

#include "Meta/serialize/operations.h"

#include "entity.h"

#include "../scenemanager.h"

#include "entitycomponentlistbase.h"

#include "entitycomponentptr.h"

#include "entitycomponentbase.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        void entityComponentHelperWrite(Serialize::FormattedSerializeStream &out, const EntityComponentHandle<EntityComponentBase> &index, const char *name, const SceneManager *mgr)
        {
            write(out, mgr->entityComponentList(index.mType).get(index)->entity(), name);
        }

        Serialize::StreamResult entityComponentHelperRead(Serialize::FormattedSerializeStream &in, EntityComponentHandle<EntityComponentBase> &index, const char *name, SceneManager *mgr)
        {
            Entity *entity;
            STREAM_PROPAGATE_ERROR(read(in, entity, name));
            if (entity)
                index = entity->getComponent(index.mType).handle();
            return {};
        }

        void entityComponentOwningHelperWrite(Serialize::FormattedSerializeStream &out, const EntityComponentHandle<EntityComponentBase> &index, const char *name, CallerHierarchyBasePtr hierarchy)
        {
            const SceneContainer *container = hierarchy;
            container->sceneMgr().entityComponentList(index.mType).writeState(index, out, name, hierarchy);
        }

        Serialize::StreamResult entityComponentOwningHelperRead(Serialize::FormattedSerializeStream &in, const EntityComponentHandle<EntityComponentBase> &index, const char *name, CallerHierarchyBasePtr hierarchy)
        {
            SceneContainer *container = hierarchy;
            return container->sceneMgr().entityComponentList(index.mType).readState(index, in, name, hierarchy);
        }

        Serialize::StreamResult entityComponentOwningHelperApplyMap(Serialize::FormattedSerializeStream &in, EntityComponentHandle<EntityComponentBase> &index, bool success, CallerHierarchyBasePtr hierarchy)
        {
            SceneContainer *container = hierarchy;
            return container->sceneMgr().entityComponentList(index.mType).applyMap(index, in, success, hierarchy);
        }

        void entityComponentOwningHelperSetSynced(EntityComponentHandle<EntityComponentBase> &index, bool synced, CallerHierarchyBasePtr hierarchy)
        {
            Entity *entity = hierarchy;
            entity->sceneMgr().entityComponentList(index.mType).setSynced(index, synced);
        }

        void entityComponentOwningHelperSetActive(EntityComponentHandle<EntityComponentBase> &index, bool active, bool existenceChanged, CallerHierarchyBasePtr hierarchy)
        {
            Entity *entity = hierarchy;
            entity->sceneMgr().entityComponentList(index.mType).setActive(index, active, existenceChanged);
        }

    }
}
}
