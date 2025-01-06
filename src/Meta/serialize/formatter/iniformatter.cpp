#include "../../metalib.h"

#include "iniformatter.h"

#include "../primitivetypes.h"

namespace Engine {
namespace Serialize {

    static std::array<std::ctype<char>::mask, 256> sTable = generateMask(~ctype::space, { { '\n', ctype::space, ~0 } });    
    static std::locale sLocale { std::locale {}, new ctype { sTable.data() } };

    IniFormatter::IniFormatter()
        : Formatter(false)
    {
    }

    void IniFormatter::setupStream(std::iostream &inout)
    {
        inout >> std::boolalpha;
        inout >> std::noskipws;
        inout << std::boolalpha;

        inout.imbue(sLocale);
    }

    void IniFormatter::beginPrimitiveWrite(const char *name, uint8_t typeId)
    {
        mStream << name << "=";
    }

    StreamResult IniFormatter::beginPrimitiveRead(const char *name, uint8_t typeId)
    {
        STREAM_PROPAGATE_ERROR(mStream.skipWs(true));
        std::string prefix;
        STREAM_PROPAGATE_ERROR(mStream.readUntil(prefix, "="));
        if (name && StringUtil::substr(prefix, 0, -1) != name)
            return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected '" << name << "', Got '" << StringUtil::substr(prefix, 0, -1) << "'";
        return {};
    }

    void IniFormatter::endPrimitiveWrite(const char *name, uint8_t typeId)
    {
        mStream << "\n";
    }

    StreamResult IniFormatter::endPrimitiveRead(const char *name, uint8_t typeId)
    {
        return {};
    }

    StreamResult IniFormatter::lookupFieldName(std::string &name)
    {
        STREAM_PROPAGATE_ERROR(mStream.peekUntil(name, "="));
        if (!name.empty())
            name = StringUtil::substr(name, 0, -1);
        return {};
    }

    void IniFormatter::beginContainerWrite(const char *name, uint32_t size)
    {
        if (size != std::numeric_limits<uint32_t>::max()) {
            mStream << "size=" << size << "\n";
        }
    }

    StreamResult IniFormatter::beginContainerRead(const char *name, bool sized)
    {
        uint32_t size = 0;
        if (sized) {
            STREAM_PROPAGATE_ERROR(mStream.skipWs(true));
            FORMATTER_EXPECT("size=");
            STREAM_PROPAGATE_ERROR(read(size));
        }
        mContainerSizes.push(size);
        return {};
    }

    StreamResult IniFormatter::endContainerRead(const char *name)
    {
        assert(mContainerSizes.top() == 0);
        mContainerSizes.pop();
        return {};
    }

    bool IniFormatter::hasContainerItem()
    {
        if (mContainerSizes.top() > 0) {
            --mContainerSizes.top();
            return true;
        }
        return false;
    }

}
}