#pragma once

#include "../hierarchy/serializableunitptr.h"

namespace Engine {
namespace Serialize {

    struct META_EXPORT SerializeStreamData {
        SerializeStreamData(SerializeManager &mgr, ParticipantId id);
        SerializeStreamData(SerializeStreamData &&) = delete;
        virtual ~SerializeStreamData();

        void setManager(SerializeManager *mgr);
        SerializeManager *manager();
        bool isMaster();

        ParticipantId id() const;
        void setId(ParticipantId id);

        SerializableUnitList &serializableList();
        friend struct SerializableListHolder;

        SerializableUnitMap &serializableMap();
        friend struct SerializableMapHolder;

    private:
        SerializeManager *mManager = nullptr;
        ParticipantId mId = 0;

        SerializableUnitMap mSerializableMap;
        SerializableUnitList mSerializableList;
    };

}
}