#include "../../metalib.h"

#include "xmlformatter.h"

#include "../primitivetypes.h"

namespace Engine {
namespace Serialize {

    static std::array<std::ctype<char>::mask, 256> sTableQuote = generateMask(~ctype::space, { { '"', ctype::space, ~0 } });    
    static std::locale sLocaleQuote { std::locale {}, new ctype {sTableQuote.data()} };
    static std::array<std::ctype<char>::mask, 256> sTableBracket = generateMask(~ctype::space, { { '<', ctype::space, ~0 } });
    static std::locale sLocaleBracket { std::locale {}, new ctype {sTableBracket.data()} };

    XMLFormatter::XMLFormatter()
        : Formatter(false, true)
    {
    }

    void XMLFormatter::setupStream(std::iostream &inout)
    {
        inout >> std::boolalpha;
        inout << std::boolalpha;
    }

    void XMLFormatter::beginExtendedWrite(const char *name, size_t count)
    {
        if (!mCurrentExtended) {
            if (!name)
                name = "Item";
            mStream << indent() << "<" << name;
            mCurrentExtended = true;
        }
        assert(mCurrentExtendedCount == 0);
        assert(count > 0);
        mCurrentExtendedCount = count;
    }

    StreamResult XMLFormatter::beginExtendedRead(const char *name, size_t count)
    {
        if (!mCurrentExtended) {
            if (name) {
                std::string prefix;
                STREAM_PROPAGATE_ERROR(mStream.readN(prefix, strlen(name) + 1));
                if (prefix != "<" + std::string(name))
                    return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected extended opening tag '<" << name << "'";
            } else {
                std::string prefix;
                STREAM_PROPAGATE_ERROR(mStream.readUntil(prefix, " >"));
                if (!StringUtil::startsWith(prefix, "<"))
                    return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected extended opening tag, found: " << prefix;
                if (prefix.size() <= 1)
                    return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected extended opening tag";
            }
            mCurrentExtended = true;
        }
        assert(mCurrentExtendedCount == 0);
        assert(count > 0);
        mCurrentExtendedCount = count;
        return {};
    }

    void XMLFormatter::beginCompoundWrite(const char *name)
    {
        if (!mCurrentExtended) {
            if (!name)
                name = "Item";
            mStream << indent() << "<" << name;
        } else {
            assert(mCurrentExtendedCount == 0);
        }
        mStream << ">\n";
        mCurrentExtended = false;
        ++mLevel;
    }

    StreamResult XMLFormatter::beginCompoundRead(const char *name)
    {
        if (!mCurrentExtended) {
            std::string prefix;
            STREAM_PROPAGATE_ERROR(mStream.readUntil(prefix, ">"));
            if (name && prefix != "<" + std::string(name) + ">")
                return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected opening tag <" << name << ">, found: " << prefix;
            mStream.seek(mStream.tell() - off_type { 1 });
        } else {
            assert(mCurrentExtendedCount == 0);
            STREAM_PROPAGATE_ERROR(prefetchAttributes());
        }
        FORMATTER_EXPECT(">");
        mCurrentExtended = false;
        return {};
    }
    void XMLFormatter::endCompoundWrite(const char *name)
    {
        --mLevel;
        if (!name)
            name = "Item";
        mStream << indent() << "</" << name << ">\n";
    }

    StreamResult XMLFormatter::endCompoundRead(const char *name)
    {
        std::string prefix;
        STREAM_PROPAGATE_ERROR(mStream.readUntil(prefix, ">"));
        if (name && prefix != "</" + std::string(name) + ">")
            return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected closing tag '</" << name << ">', found: " << prefix;
        if (!StringUtil::startsWith(prefix, "</"))
            return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected closing tag";
        return {};
    }

    void XMLFormatter::beginPrimitiveWrite(const char *name, uint8_t typeId)
    {
        if (!name)
            name = "Element";
        if (mCurrentExtendedCount > 0) {
            assert(mCurrentExtended);
            --mCurrentExtendedCount;
            mStream << " " << name << "=";
            if (typeId == Serialize::PrimitiveTypeIndex_v<std::string> || typeId == Serialize::PrimitiveTypeIndex_v<ByteBuffer>)
                mStream << "\"";
        } else {
            if (!mCurrentExtended) {
                mStream << indent() << "<" << name;
            } else {
                mCurrentExtended = false;
            }
            mStream << ">";
        }
    }

    StreamResult XMLFormatter::beginPrimitiveRead(const char *name, uint8_t typeId)
    {
        if (mCurrentExtendedCount > 0) {
            assert(mCurrentExtended);
            --mCurrentExtendedCount;
            auto it = mPrefetchedAttributes.find(name);
            if (it != mPrefetchedAttributes.end()) {
                mExtendedLookupPos = mStream.tell();
                mStream.seek(it->second);
                mPrefetchedAttributes.erase(it);
            } else {
                STREAM_PROPAGATE_ERROR(prefetchAttributes(name));
            }
            if (typeId == Serialize::PrimitiveTypeIndex_v<std::string> || typeId == Serialize::PrimitiveTypeIndex_v<ByteBuffer>) {
                FORMATTER_EXPECT("\"");
                pushLocale(sLocaleQuote, false);
            }
        } else {
            if (!mCurrentExtended) {
                std::string prefix;
                STREAM_PROPAGATE_ERROR(mStream.peekUntil(prefix, ">"));
                if (prefix.size() <= 1)
                    return STREAM_PARSE_ERROR(mStream, mBinary) << "Syntax error";
                if (name && prefix != "<" + std::string(name) + ">")
                    return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected: '" << name << "'";
                if (!name && prefix[1] == '/')
                    return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected unnamed primitive";
                STREAM_PROPAGATE_ERROR(mStream.readN(prefix, prefix.size() - 1));
            } else {
                mCurrentExtended = false;
            }
            FORMATTER_EXPECT(">");

            if (typeId == Serialize::PrimitiveTypeIndex_v<std::string> || typeId == Serialize::PrimitiveTypeIndex_v<ByteBuffer> || typeId == Serialize::PrimitiveTypeIndex_v<EnumTag> || typeId == Serialize::PrimitiveTypeIndex_v<std::chrono::nanoseconds>) {
                pushLocale(sLocaleBracket, false);                
            }
        }
        return {};
    }

    void XMLFormatter::endPrimitiveWrite(const char *name, uint8_t typeId)
    {
        if (!name)
            name = "Element";
        if (mCurrentExtended) {
            if (typeId == Serialize::PrimitiveTypeIndex_v<std::string> || typeId == Serialize::PrimitiveTypeIndex_v<ByteBuffer>)
                mStream << "\"";
        } else
            mStream << "</" << name << ">\n";
    }

    StreamResult XMLFormatter::endPrimitiveRead(const char *name, uint8_t typeId)
    {
        if (mCurrentExtended) {
            if (mExtendedLookupPos != -1) {
                mStream.seek(mExtendedLookupPos);
                mExtendedLookupPos = -1;
            }
            if (typeId == Serialize::PrimitiveTypeIndex_v<std::string> || typeId == Serialize::PrimitiveTypeIndex_v<ByteBuffer>) {                
                popLocale();
                FORMATTER_EXPECT("\"");
            }
        } else {
            const char *cPrefix = "</";
            if (typeId == Serialize::PrimitiveTypeIndex_v<std::string> || typeId == Serialize::PrimitiveTypeIndex_v<ByteBuffer> || typeId == Serialize::PrimitiveTypeIndex_v<EnumTag> || typeId == Serialize::PrimitiveTypeIndex_v<std::chrono::nanoseconds>) {
                popLocale();
            }
            std::string prefix;
            STREAM_PROPAGATE_ERROR(mStream.readUntil(prefix, ">"));
            if (prefix.size() <= 1)
                return STREAM_PARSE_ERROR(mStream, mBinary) << "Syntax error";
            if (name && prefix != cPrefix + std::string(name) + ">")
                return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected: '" << name << "'";
        }
        return {};
    }

    StreamResult XMLFormatter::lookupFieldName(std::string &name)
    {
        name.clear();
        std::string dummy;
        STREAM_PROPAGATE_ERROR(mStream.peekUntil(dummy, "> "));
        if (!dummy.empty()) {
            if (dummy[0] != '<')
                return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected: '<'";
            if (dummy[1] == '/')
                return {};
            name = StringUtil::substr(dummy, 1, -1);
        }
        return {};
    }

    void XMLFormatter::beginContainerWrite(const char *name, uint32_t size)
    {
        beginCompoundWrite(name);
    }

    void XMLFormatter::endContainerWrite(const char *name)
    {
        endCompoundWrite(name);
    }

    StreamResult XMLFormatter::beginContainerRead(const char *name, bool sized)
    {
        return beginCompoundRead(name);
    }

    StreamResult XMLFormatter::endContainerRead(const char *name)
    {
        return endCompoundRead(name);
    }

    bool XMLFormatter::hasContainerItem()
    {
        std::string prefix;
        if (mStream.peekN(prefix, 2).mState != StreamState::OK)
            return false;
        if (prefix[0] != '<')
            throw 0;
        return prefix[1] != '/';
    }

    std::string XMLFormatter::indent()
    {
        return std::string(4 * mLevel, ' ');
    }

    StreamResult XMLFormatter::prefetchAttributes(const char *name)
    {
        std::string prefix;
        STREAM_PROPAGATE_ERROR(mStream.peekN(prefix, 1));
        while (prefix != ">") {
            STREAM_PROPAGATE_ERROR(mStream.readUntil(prefix, "="));
            if (name && prefix == name + "="s)
                return {};
            mPrefetchedAttributes.try_emplace(std::string { StringUtil::substr(prefix, 0, -1) }, mStream.tell());
            STREAM_PROPAGATE_ERROR(skipValue());
            STREAM_PROPAGATE_ERROR(mStream.peekN(prefix, 1));
        }
        if (name)
            return STREAM_PARSE_ERROR(mStream, mBinary) << "Missing attribute '" << name << "'";
        return {};
    }

    StreamResult XMLFormatter::skipValue()
    {
        std::string next;
        STREAM_PROPAGATE_ERROR(mStream.peekN(next, 1));
        if (next == "\"") {
            STREAM_PROPAGATE_ERROR(mStream.readN(next, 1));
            STREAM_PROPAGATE_ERROR(mStream.readUntil(next, "\""));
        } else {
            float dummy; //TODO Is this catching all cases?
            STREAM_PROPAGATE_ERROR(mStream >> dummy);
        }
        return {};
    }

}
}
