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
            assert(mData->mSerializableMap.empty());
        }
    }

    SerializableMapHolder::SerializableMapHolder(SerializableMapHolder &&other)
        : mData(std::exchange(other.mData, nullptr))
    {
    }

    SerializableMapHolder::~SerializableMapHolder()
    {
        if (mData) {
            mData->mSerializableMap.clear();
        }
    }

    SerializableListHolder::SerializableListHolder(SerializeStreamData *data)
        : mData(data)
    {
        if (mData) {
            assert(mData->mSerializableList.empty());            
        }
    }

    SerializableListHolder::SerializableListHolder(FormattedSerializeStream &in)
        : SerializableListHolder(in.data())
    {
    }

    SerializableListHolder::~SerializableListHolder()
    {
        if (mData) {
            mData->mSerializableList.clear();
        }
    }

    SerializableListHolder &SerializableListHolder::operator=(SerializableListHolder &&other)        
    {
        std::swap(mData, other.mData);
        return *this;
    }

}
}