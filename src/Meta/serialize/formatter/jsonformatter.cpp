#include "../../metalib.h"

#include "jsonformatter.h"

#include "../primitivetypes.h"

namespace Engine {
namespace Serialize {

    static std::array<std::ctype<char>::mask, 256> sTableQuote = generateMask(~ctype::space, { { '"', ctype::space, ~0 } });
    static std::locale sLocaleQuote { std::locale {}, new ctype { sTableQuote.data() } };

    JSONFormatter::JSONFormatter()
        : Formatter(false, true)
    {
        mParseLevel.emplace(false);
    }

    void JSONFormatter::setupStream(std::iostream &inout)
    {
        inout >> std::boolalpha;
        inout << std::boolalpha;
    }

    void JSONFormatter::beginExtendedWrite(const char *name, size_t count)
    {
        if (!mCurrentExtended) {
            if (mAfterItemWrite) {
                mStream << ",\n";
                mAfterItemWrite = false;
            } else if (mParseLevel.top().mIsContainer) {
                mStream << "\n";
            }
            mStream << indent();
            if (!mParseLevel.top().mIsContainer) {
                if (!name)
                    name = "Item";
                mStream << "\"" << name << "\" : ";
            }
            mParseLevel.emplace(false);
            mStream << "{\n";
            mCurrentExtended = true;
            ++mLevel;
            mStream << indent() << "\"__extended\" : {\n";
            ++mLevel;
        }
        assert(mCurrentExtendedCount == 0);
        assert(count > 0);
        mCurrentExtendedCount = count;
    }

    StreamResult JSONFormatter::beginExtendedRead(const char *name, size_t count)
    {
        if (!mCurrentExtended) {
            if (!mParseLevel.top().mIsContainer) {
                auto it = mParseLevel.top().mPrefetchedFields.find(name);
                if (it != mParseLevel.top().mPrefetchedFields.end()) {
                    mParseLevel.top().mLookupPos = mStream.tell();
                    mStream.seek(it->second);
                    mParseLevel.top().mPrefetchedFields.erase(it);
                } else {
                    if (mAfterItemRead) {
                        FORMATTER_EXPECT(",");
                        mAfterItemRead = false;
                    }
                    STREAM_PROPAGATE_ERROR(readFieldName(name));
                }
            } else {
                if (mAfterItemRead) {
                    FORMATTER_EXPECT(",");
                    mAfterItemRead = false;
                }
            }
            mParseLevel.emplace(false);
            FORMATTER_EXPECT("{");
            mCurrentExtended = true;
            STREAM_PROPAGATE_ERROR(readFieldName("__extended"));
            FORMATTER_EXPECT("{");
        }
        assert(mCurrentExtendedCount == 0);
        assert(count > 0);
        mCurrentExtendedCount = count;
        return {};
    }

    void JSONFormatter::beginCompoundWrite(const char *name)
    {
        if (!mCurrentExtended) {
            if (mAfterItemWrite) {
                mStream << ",\n";
                mAfterItemWrite = false;
            } else if (mParseLevel.top().mIsContainer) {
                mStream << "\n";
            }
            mStream << indent();
            if (!mParseLevel.top().mIsContainer) {
                if (!name)
                    name = "Item";
                mStream << "\"" << name << "\" : ";
            }
            mParseLevel.emplace(false);
            mStream << "{\n";
            ++mLevel;
        } else {
            assert(mCurrentExtendedCount == 0);
            --mLevel;
            mStream << "\n"
                    << indent() << "}";
            mAfterItemWrite = true;
        }
        mCurrentExtended = false;
    }

    StreamResult JSONFormatter::beginCompoundRead(const char *name)
    {
        if (!mCurrentExtended) {
            if (!mParseLevel.top().mIsContainer) {
                auto it = mParseLevel.top().mPrefetchedFields.find(name);
                if (it != mParseLevel.top().mPrefetchedFields.end()) {
                    mParseLevel.top().mLookupPos = mStream.tell();
                    mStream.seek(it->second);
                    mParseLevel.top().mPrefetchedFields.erase(it);
                } else {
                    if (mAfterItemRead) {
                        FORMATTER_EXPECT(",");
                        mAfterItemRead = false;
                    }
                    STREAM_PROPAGATE_ERROR(readFieldName(name));
                }
            } else {
                if (mAfterItemRead) {
                    FORMATTER_EXPECT(",");
                    mAfterItemRead = false;
                }
            }

            mParseLevel.emplace(false);
            FORMATTER_EXPECT("{");
        } else {
            assert(mCurrentExtendedCount == 0);
            FORMATTER_EXPECT("}");
            mAfterItemRead = true;
        }
        mCurrentExtended = false;
        return {};
    }

    void JSONFormatter::endCompoundWrite(const char *name)
    {
        if (mAfterItemWrite) {
            mAfterItemWrite = false;
        }
        --mLevel;
        mStream << "\n"
                << indent() << "}";
        assert(!mParseLevel.top().mIsContainer);
        mParseLevel.pop();
        mAfterItemWrite = true;
        mLastPrimitive = false;
    }

    StreamResult JSONFormatter::endCompoundRead(const char *name)
    {
        if (mAfterItemRead) {
            mAfterItemRead = false;
        }
        FORMATTER_EXPECT("}");
        assert(!mParseLevel.top().mIsContainer);
        mParseLevel.pop();
        if (mParseLevel.top().mLookupPos != -1) {
            mStream.seek(mParseLevel.top().mLookupPos);
            mParseLevel.top().mLookupPos = -1;
        }
        mAfterItemRead = true;

        return {};
    }

    void JSONFormatter::beginPrimitiveWrite(const char *name, uint8_t typeId)
    {
        if (mAfterItemWrite) {
            mStream << ",";
            if (!mLastPrimitive || !mParseLevel.top().mIsContainer)
                mStream << "\n";
            mAfterItemWrite = false;
        }
        if (mCurrentExtendedCount > 0) {
            assert(mCurrentExtended);
            --mCurrentExtendedCount;
        }
        if (!mParseLevel.top().mIsContainer) {
            if (!name)
                name = "Element";
            mStream << indent() << "\"" << name << "\" : ";
        }
        if (typeId == Serialize::PrimitiveTypeIndex_v<std::string> || typeId == Serialize::PrimitiveTypeIndex_v<Memory::ByteBuffer>)
            mStream << "\"";
    }

    StreamResult JSONFormatter::beginPrimitiveRead(const char *name, uint8_t typeId)
    {
        if (mCurrentExtendedCount > 0) {
            assert(mCurrentExtended);
            --mCurrentExtendedCount;
        }
        if (!mParseLevel.top().mIsContainer) {
            auto it = mParseLevel.top().mPrefetchedFields.find(name);
            if (it != mParseLevel.top().mPrefetchedFields.end()) {
                mParseLevel.top().mLookupPos = mStream.tell();
                mStream.seek(it->second);
                mParseLevel.top().mPrefetchedFields.erase(it);
            } else {
                if (mAfterItemRead) {
                    FORMATTER_EXPECT(",");
                    mAfterItemRead = false;
                }
                STREAM_PROPAGATE_ERROR(readFieldName(name));
            }
        } else {
            if (mAfterItemRead) {
                FORMATTER_EXPECT(",");
                mAfterItemRead = false;
            }
        }
        if (typeId == Serialize::PrimitiveTypeIndex_v<std::string> || typeId == Serialize::PrimitiveTypeIndex_v<Memory::ByteBuffer>) {
            FORMATTER_EXPECT("\"");
            pushLocale(sLocaleQuote, false);
        }
        return {};
    }

    void JSONFormatter::endPrimitiveWrite(const char *name, uint8_t typeId)
    {
        if (typeId == Serialize::PrimitiveTypeIndex_v<std::string> || typeId == Serialize::PrimitiveTypeIndex_v<Memory::ByteBuffer>)
            mStream << "\"";
        mAfterItemWrite = true;
        mLastPrimitive = true;
    }

    StreamResult JSONFormatter::endPrimitiveRead(const char *name, uint8_t typeId)
    {
        if (typeId == Serialize::PrimitiveTypeIndex_v<std::string> || typeId == Serialize::PrimitiveTypeIndex_v<Memory::ByteBuffer>) {
            popLocale();
            FORMATTER_EXPECT("\"");
        }
        if (mParseLevel.top().mLookupPos != -1) {
            mStream.seek(mParseLevel.top().mLookupPos);
            mParseLevel.top().mLookupPos = -1;
        }
        mAfterItemRead = true;
        return {};
    }

    StreamResult JSONFormatter::readFieldName(const char *name)
    {
        std::string prefix;
        STREAM_PROPAGATE_ERROR(mStream.readUntil(prefix, ":"));
        std::string_view trimmed = StringUtil::trim(StringUtil::substr(prefix, 0, -1));
        while (name && StringUtil::substr(trimmed, 1, -1) != name) {
            mParseLevel.top().mPrefetchedFields.try_emplace(std::string { StringUtil::substr(trimmed, 1, -1) }, mStream.tell());
            STREAM_PROPAGATE_ERROR(skipObject());
            FORMATTER_EXPECT(",");
            STREAM_PROPAGATE_ERROR(mStream.readUntil(prefix, ":"));
            trimmed = { prefix.c_str(), prefix.size() - 1 };
            trimmed = StringUtil::trim(trimmed);
        }
        if (!StringUtil::startsWith(trimmed, "\"") || !StringUtil::endsWith(trimmed, "\""))
            return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected key in '\"\"'";
        if (trimmed.size() <= 1)
            return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected key";
        return {};
    }

    StreamResult JSONFormatter::lookupFieldName(std::string &name)
    {
        name.clear();
        pos_type pos = mStream.tell();
        std::string controlChar;
        STREAM_PROPAGATE_ERROR(mStream.readN(controlChar, 1));
        if (controlChar == "}") {
            mStream.seek(pos);
            return {};
        }
        if (mAfterItemRead) {
            if (controlChar != ",")
                throw 0;
        } else {
            mStream.seek(pos);
        }
        std::string prefix;
        STREAM_PROPAGATE_ERROR(mStream.readUntil(prefix, ":"));
        std::string_view trimmed { prefix.c_str(), prefix.size() - 1 };
        trimmed = StringUtil::trim(trimmed);
        if (!StringUtil::startsWith(trimmed, "\"") || !StringUtil::endsWith(trimmed, "\""))
            throw 0;
        if (trimmed.size() <= 1)
            throw 0;
        mStream.seek(pos);
        name = StringUtil::substr(trimmed, 1, -1);
        return {};
    }

    void JSONFormatter::beginContainerWrite(const char *name, uint32_t size)
    {
        if (mAfterItemWrite) {
            mStream << ",\n";
            mAfterItemWrite = false;
        }
        if (!name)
            name = "Item";
        mStream << indent() << "\"" << name << "\" : [";
        ++mLevel;
        mParseLevel.emplace(true);
    }

    StreamResult JSONFormatter::beginContainerRead(const char *name, bool sized)
    {
        if (mAfterItemRead) {
            FORMATTER_EXPECT(",");
            mAfterItemRead = false;
        }
        STREAM_PROPAGATE_ERROR(readFieldName(name));
        std::string bracket;
        STREAM_PROPAGATE_ERROR(sized ? mStream.peekN(bracket, 1) : mStream.readN(bracket, 1));
        if (bracket != "[")
            return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected '['";
        mParseLevel.emplace(true);
        return {};
    }

    void JSONFormatter::endContainerWrite(const char *name)
    {
        assert(mParseLevel.top().mIsContainer);
        mParseLevel.pop();
        if (mAfterItemWrite) {
            mAfterItemWrite = false;
        }
        --mLevel;
        if (!mLastPrimitive)
            mStream << "\n"
                    << indent();
        mStream << "]";
        mAfterItemWrite = true;
    }

    StreamResult JSONFormatter::endContainerRead(const char *name)
    {
        assert(mParseLevel.top().mIsContainer);
        mParseLevel.pop();
        if (mAfterItemRead) {
            mAfterItemRead = false;
        }
        FORMATTER_EXPECT("]");
        mAfterItemRead = true;
        return {};
    }

    bool JSONFormatter::hasContainerItem()
    {
        std::string prefix;
        if (mStream.peekN(prefix, 1).mState != StreamState::OK)
            return false;
        if (prefix[0] == '[') {
            if (mStream.readN(prefix, 1).mState != StreamState::OK)
                return false;
            if (mStream.peekN(prefix, 1).mState != StreamState::OK)
                return false;
            return prefix != "]";
        }
        if (prefix[0] == ',')
            return true;
        if (prefix[0] != ']')
            throw 0;
        return false;
    }

    void JSONFormatter::beginMessageWrite()
    {
        assert(!mAfterItemWrite && mParseLevel.empty());
    }

    void JSONFormatter::endMessageWrite()
    {
        assert(mAfterItemWrite && mParseLevel.empty());
        mAfterItemWrite = false;
    }

    StreamResult JSONFormatter::beginMessageRead()
    {
        assert(!mAfterItemRead && mParseLevel.empty());
        return {};
    }

    StreamResult JSONFormatter::endMessageRead()
    {
        if (!mAfterItemRead || !mParseLevel.empty())
            return STREAM_INTEGRITY_ERROR(mStream, mBinary) << "Message ended unexpectedly!";
        mAfterItemRead = false;
        return {};
    }

    std::string JSONFormatter::indent()
    {
        return std::string(4 * mLevel, ' ');
    }

    StreamResult JSONFormatter::skipObject()
    {
        std::string next;
        STREAM_PROPAGATE_ERROR(mStream.readN(next, 1));
        switch (next[0]) {
        case '[': {
            do {
                STREAM_PROPAGATE_ERROR(skipObject());
                STREAM_PROPAGATE_ERROR(mStream.readN(next, 1));
            } while (next == ",");
            if (next != "]")
                return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected ']', Found '" << next << "'.";
        } break;
        case '{': {
            do {
                STREAM_PROPAGATE_ERROR(readFieldName(nullptr));
                STREAM_PROPAGATE_ERROR(skipObject());
                STREAM_PROPAGATE_ERROR(mStream.readN(next, 1));
            } while (next == ",");
            if (next != "}")
                return STREAM_PARSE_ERROR(mStream, mBinary) << "Expected '}', Found '" << next << "'.";
        } break;
        default: {
            std::string value;
            STREAM_PROPAGATE_ERROR(mStream.readUntil(value, "}],"));
            mStream.seek(mStream.tell() - off_type { 1 });
        }
        }
        return {};
    }

}
}
