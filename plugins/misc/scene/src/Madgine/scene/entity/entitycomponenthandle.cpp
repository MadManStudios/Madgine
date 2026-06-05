#include "../../scenelib.h"

#include "entitycomponenthandle.h"

#include "Meta/serialize/operations.h"

#include "../scenemanager.h"
#include "entity.h"
#include "entitycomponentbase.h"
#include "entitycomponentlistbase.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        void entityComponentHelperWrite(Serialize::CallerHierarchyFormattedSerializeStream out, const EntityComponentHandle &index, const char *name)
        {
            const SceneContainer *container = out.mHierarchy;
            container->sceneMgr().entityComponentList(index.mType).writeState(index.mComponent, out, name);
        }

        Serialize::StreamResult entityComponentHelperRead(Serialize::CallerHierarchyFormattedSerializeStream in, const EntityComponentHandle &index, const char *name)
        {
            SceneContainer *container = in.mHierarchy;
            return container->sceneMgr().entityComponentList(index.mType).readState(index.mComponent, in, name);
        }

        Serialize::StreamResult entityComponentHelperApplyMap(Serialize::CallerHierarchyFormattedSerializeStream in, EntityComponentHandle &index, bool success)
        {
            SceneContainer *container = in.mHierarchy;
            return container->sceneMgr().entityComponentList(index.mType).applyMap(index.mComponent, in, success);
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

        std::string_view EntityComponentHandle::name() const
        {
            return EntityComponentRegistry::sComponentName(mType);
        }

        Reflect::ScopePtr EntityComponentHandle::getTyped() const
        {
            return { &mComponent, *EntityComponentRegistry::get(mType).mType };
        }

    }
}
}
