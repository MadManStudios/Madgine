#include "../../scenelib.h"

#include "entitycomponentlistbase.h"

#include "Meta/serialize/operations.h"

namespace Engine {
namespace Scene {
    namespace Entity {
        
        Serialize::StreamResult EntityComponentListBase::readState(const EntityComponentHandle<EntityComponentBase> &index, Serialize::FormattedSerializeStream &in, const char *name, CallerHierarchyBasePtr hierarchy)
        {
            return getSerialized(index).readState(in, name, hierarchy);
        }

        void EntityComponentListBase::writeState(const EntityComponentHandle<EntityComponentBase> &index, Serialize::FormattedSerializeStream &out, const char *name, CallerHierarchyBasePtr hierarchy) const
        {
            getSerialized(index).writeState(out, name, hierarchy);
        }

        Serialize::StreamResult EntityComponentListBase::applyMap(const EntityComponentHandle<EntityComponentBase> &index, Serialize::FormattedSerializeStream &in, bool success, CallerHierarchyBasePtr hierarchy)
        {            
            return getSerialized(index).applyMap(in, success, hierarchy);
        }

    }
}
}