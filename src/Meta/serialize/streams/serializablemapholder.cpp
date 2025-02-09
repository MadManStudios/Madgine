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

    SerializableMapHolder::SerializableMapHolder(SerializableMapHolder &&other)
        : mMap(std::move(other.mMap))
        , mData(std::exchange(other.mData, nullptr))
    {
        if (mData) {
            assert(mData->mSerializableMap == &other.mMap);
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

    SerializableListHolder::SerializableListHolder(SerializeStreamData *data)
        : mData(data)
    {
        if (mData) {
            assert(!mData->mSerializableList);
            mData->mSerializableList = &mList;
        }
    }

    SerializableListHolder::SerializableListHolder(FormattedSerializeStream &in)
        : SerializableListHolder(in.data())
    {
    }

    SerializableListHolder::~SerializableListHolder()
    {
        if (mData) {
            assert(mData->mSerializableList == &mList);
            mData->mSerializableList = nullptr;
        }
    }

    SerializableListHolder &SerializableListHolder::operator=(SerializableListHolder &&other)        
    {
        std::swap(mData, other.mData);
        std::swap(mList, other.mList);
        if (mData) {
            assert(mData->mSerializableList == &other.mList);
            mData->mSerializableList = &mList;
        }
        if (other.mData) {
            assert(other.mData->mSerializableList == &mList);
            other.mData->mSerializableList = &other.mList;
        }
        return *this;
    }

}
}