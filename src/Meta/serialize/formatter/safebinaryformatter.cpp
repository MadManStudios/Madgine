#include "../../metalib.h"

#include "safebinaryformatter.h"

namespace Engine {
namespace Serialize {

    constexpr const uint16_t SERIALIZE_MAGIC_NUMBER = 0x5AFE;

    SafeBinaryFormatter::SafeBinaryFormatter()
        : Formatter(true)
    {
    }

    void SafeBinaryFormatter::beginPrimitiveWrite(const char *name, uint8_t typeId)
    {
        mStream.write<uint16_t>(SERIALIZE_MAGIC_NUMBER + typeId);
    }

    StreamResult SafeBinaryFormatter::beginPrimitiveRead(const char *name, uint8_t typeId)
    {
        uint16_t type;
        STREAM_PROPAGATE_ERROR(mStream.read(type));
        type -= SERIALIZE_MAGIC_NUMBER;
        if (type != typeId)
            return STREAM_INTEGRITY_ERROR(mStream, mBinary) << "Invalid type-id encountered!";
        return {};
    }

    void SafeBinaryFormatter::beginContainerWrite(const char *name, uint32_t size)
    {
        if (size != std::numeric_limits<uint32_t>::max())
            mStream.write(size);
    }

    StreamResult SafeBinaryFormatter::beginContainerRead(const char *name, bool sized)
    {
        uint32_t size = 0;
        if (sized) {
            STREAM_PROPAGATE_ERROR(mStream.read(size));
        }
        mContainerSizes.push(size);
        return {};
    }

    StreamResult SafeBinaryFormatter::endContainerRead(const char *name)
    {
        assert(mContainerSizes.top() == 0);
        mContainerSizes.pop();
        return {};
    }

    bool SafeBinaryFormatter::hasContainerItem()
    {
        if (mContainerSizes.top() > 0) {
            --mContainerSizes.top();
            return true;
        }
        return false;
    }

}
}