#include "../../metalib.h"

#include "serializablemapholder.h"

#include "formattedserializestream.h"

#include "serializestreamdata.h"


#include "../hierarchy/serializableunitptr.h"


namespace Engine {
namespace Serialize {

    SerializableMapHolder::SerializableMapHolder(FormattedSerializeStream &out)
        : mData(out.data())
    {
        if (mData) {
            assert(!mData->mSerializableMap);
            mData->mSerializableMap = &mMap;
        }
    }

    SerializableMapHolder::~SerializableMapHolder()
    {
        if (mData) {
            assert(mData->mSerializableMap == &mMap);
            mData->mSerializableMap = nullptr;
        }
    }

    SerializableListHolder::SerializableListHolder(FormattedSerializeStream &in)
        : mData(in.data())
    {
        if (mData) {
            assert(!mData->mSerializableList);
            mData->mSerializableList = &mList;
        }
    }

    SerializableListHolder::~SerializableListHolder()
    {
        if (mData) {
            assert(mData->mSerializableList == &mList);
            mData->mSerializableList = nullptr;
        }
    }

}
}