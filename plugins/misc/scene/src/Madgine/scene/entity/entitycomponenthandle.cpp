#include "../../scenelib.h"

#include "entitycomponenthandle.h"

#include "Meta/serialize/operations.h"

#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "../scenemanager.h"
#include "entity.h"
#include "entitycomponentbase.h"
#include "entitycomponentlistbase.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        void entityComponentHelperWrite(Serialize::FormattedSerializeStream &out, const EntityComponentHandle &index, const char *name, Serialize::ContextPtr context)
        {
            const SceneContainer *container = context_get<const SceneContainer>(context);
            container->sceneMgr().entityComponentList(index.mType).writeState(index.mComponent, out, name, context);
        }

        Serialize::StreamResult entityComponentHelperRead(Serialize::FormattedSerializeStream &in, const EntityComponentHandle &index, const char *name, Serialize::ContextPtr context)
        {
            SceneContainer *container = context_get<SceneContainer>(context);
            return container->sceneMgr().entityComponentList(index.mType).readState(index.mComponent, in, name, context);
        }

        Serialize::StreamResult entityComponentHelperApplyMap(Serialize::FormattedSerializeStream &in, EntityComponentHandle &index, bool success, Serialize::ContextPtr context)
        {
            SceneContainer *container = context_get<SceneContainer>(context);
            return container->sceneMgr().entityComponentList(index.mType).applyMap(index.mComponent, in, success);
        }

        void entityComponentHelperSetSynced(EntityComponentHandle &index, bool synced, Serialize::ContextPtr context)
        {
            Entity *entity = context_get<Entity>(context);
            entity->sceneMgr().entityComponentList(index.mType).setSynced(index.mComponent, synced);
        }

        void entityComponentHelperSetActive(EntityComponentHandle &index, bool active, bool existenceChanged, Serialize::ContextPtr context)
        {
            Entity *entity = context_get<Entity>(context);
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
