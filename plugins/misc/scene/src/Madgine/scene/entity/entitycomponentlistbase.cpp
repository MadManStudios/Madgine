#include "../../scenelib.h"

#include "entitycomponentlistbase.h"

#include "Meta/serialize/operations.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        Serialize::StreamResult EntityComponentListBase::readState(EntityComponentBase &comp, Serialize::FormattedSerializeStream &in, const char *name)
        {
            return getSerialized(comp).readState(in, name);
        }

        void EntityComponentListBase::writeState(EntityComponentBase &comp, Serialize::FormattedSerializeStream &out, const char *name) const
        {
            getSerialized(comp).writeState(out, name);
        }

        Serialize::StreamResult EntityComponentListBase::applyMap(EntityComponentBase &comp, Serialize::FormattedSerializeStream &in, bool success)
        {
            return getSerialized(comp).applyMap(in, success);
        }

    }
}
}