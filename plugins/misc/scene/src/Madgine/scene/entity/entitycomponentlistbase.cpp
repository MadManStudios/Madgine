#include "../../scenelib.h"

#include "entitycomponentlistbase.h"

#include "Meta/serialize/operations.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        Serialize::StreamResult EntityComponentListBase::readState(EntityComponentBase &comp, Serialize::FormattedSerializeStream &in, const char *name, Serialize::ContextPtr context)
        {
            return getSerialized(comp).readState(in, name, false, context);
        }

        void EntityComponentListBase::writeState(EntityComponentBase &comp, Serialize::FormattedSerializeStream &out, const char *name, Serialize::ContextPtr context) const
        {
            getSerialized(comp).writeState(out, name, false, context);
        }

        Serialize::StreamResult EntityComponentListBase::applyMap(EntityComponentBase &comp, Serialize::FormattedSerializeStream &in, bool success)
        {
            return getSerialized(comp).applyMap(in, success);
        }

    }
}
}