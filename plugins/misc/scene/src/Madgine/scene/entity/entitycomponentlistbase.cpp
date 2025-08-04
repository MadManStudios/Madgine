#include "../../scenelib.h"

#include "entitycomponentlistbase.h"

#include "Meta/serialize/operations.h"

namespace Engine {
namespace Scene {
    namespace Entity {
        
        Serialize::StreamResult EntityComponentListBase::readState(EntityComponentBase *comp, Serialize::FormattedSerializeStream &in, const char *name, CallerHierarchyBasePtr hierarchy)
        {
            return getSerialized(comp).readState(in, name, hierarchy);
        }

        void EntityComponentListBase::writeState(EntityComponentBase *comp, Serialize::FormattedSerializeStream &out, const char *name, CallerHierarchyBasePtr hierarchy) const
        {
            getSerialized(comp).writeState(out, name, hierarchy);
        }

        Serialize::StreamResult EntityComponentListBase::applyMap(EntityComponentBase *comp, Serialize::FormattedSerializeStream &in, bool success, CallerHierarchyBasePtr hierarchy)
        {            
            return getSerialized(comp).applyMap(in, success, hierarchy);
        }

    }
}
}