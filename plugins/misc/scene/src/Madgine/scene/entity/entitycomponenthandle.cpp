#include "../../scenelib.h"
#include "entitycomponenthandle.h"

#include "Meta/serialize/operations.h"

#include "entity.h"

#include "../scenemanager.h"

#include "entitycomponentlistbase.h"

#include "entitycomponentbase.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        void entityComponentHelperWrite(Serialize::FormattedSerializeStream &out, const EntityComponentHandle &index, const char *name, CallerHierarchyBasePtr hierarchy)
        {
            const SceneContainer *container = hierarchy;
            container->sceneMgr().entityComponentList(index.mType).writeState(index.mComponent, out, name, hierarchy);
        }

        Serialize::StreamResult entityComponentHelperRead(Serialize::FormattedSerializeStream &in, const EntityComponentHandle &index, const char *name, CallerHierarchyBasePtr hierarchy)
        {
            SceneContainer *container = hierarchy;
            return container->sceneMgr().entityComponentList(index.mType).readState(index.mComponent, in, name, hierarchy);
        }

        Serialize::StreamResult entityComponentHelperApplyMap(Serialize::FormattedSerializeStream &in, EntityComponentHandle &index, bool success, CallerHierarchyBasePtr hierarchy)
        {
            SceneContainer *container = hierarchy;
            return container->sceneMgr().entityComponentList(index.mType).applyMap(index.mComponent, in, success, hierarchy);
        }

        void entityComponentHelperSetSynced(EntityComponentHandle &index, bool synced, CallerHierarchyBasePtr hierarchy)
        {
            Entity *entity = hierarchy;
            entity->sceneMgr().entityComponentList(index.mType).setSynced(index.mComponent, synced);
        }

        void entityComponentHelperSetActive(EntityComponentHandle &index, bool active, bool existenceChanged, CallerHierarchyBasePtr hierarchy)
        {
            Entity *entity = hierarchy;
            entity->sceneMgr().entityComponentList(index.mType).setActive(index.mComponent, active, existenceChanged);
        }

        std::string_view EntityComponentHandle::name() const {
            return EntityComponentRegistry::sComponentName(mType);
        }

        ScopePtr EntityComponentHandle::getTyped() const {
            return { mComponent, *EntityComponentRegistry::get(mType).mType };
        }

    }
}
}
