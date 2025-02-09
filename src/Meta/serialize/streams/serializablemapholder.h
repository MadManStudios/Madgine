#pragma once

namespace Engine {
namespace Serialize {

    struct META_EXPORT SerializableMapHolder {

        SerializableMapHolder(FormattedSerializeStream &out);     
        SerializableMapHolder(const SerializableMapHolder &) = delete;
        SerializableMapHolder(SerializableMapHolder &&);
        ~SerializableMapHolder();

        SerializableUnitMap mMap;    
        SerializeStreamData *mData = nullptr;
    };

    struct META_EXPORT SerializableListHolder {
        SerializableListHolder(SerializeStreamData *data = nullptr);
        SerializableListHolder(FormattedSerializeStream &in);
        SerializableListHolder(const SerializableListHolder &) = delete;
        SerializableListHolder(SerializableListHolder &&) = delete;
        ~SerializableListHolder();

        SerializableListHolder &operator=(SerializableListHolder &&);

        SerializableUnitList mList;
        SerializeStreamData *mData = nullptr;
    };

}
}