#include "../metalib.h"

#include "operations.h"

#include "serializemanager.h"
#include "streams/formattedserializestream.h"
#include "hierarchy/syncableunit.h"

namespace Engine {
namespace Serialize {

    StreamResult convertSyncablePtr(FormattedSerializeStream &in, UnitId id, SyncableUnitBase *&out, const SerializeTable *&type)
    {
        STREAM_PROPAGATE_ERROR(SerializeManager::convertPtr(in.stream(), id, out));
        type = out->mType;
        return {};
    }

    StreamResult convertSerializablePtr(FormattedSerializeStream &in, uint32_t id, SerializableDataPtr &out)
    {
        if (id > in.serializableList().size())
            return STREAM_INTEGRITY_ERROR(in) << "Unknown Serializable Unit-Id (" << id << ") used!";
        out = in.serializableList().at(id);
        return {};
    }
}
}
