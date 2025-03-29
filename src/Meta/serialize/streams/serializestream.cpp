#include "../../metalib.h"
#include "serializestream.h"

#include "serializestreamdata.h"

#include "Generic/bytebuffer.h"

#include "../serializemanager.h"

#include "Meta/base64/base64.h"

#include "../hierarchy/serializableunitptr.h"

namespace Engine {
namespace Serialize {

    SerializeStream::SerializeStream()
    {
    }

    SerializeStream::SerializeStream(std::unique_ptr<std::basic_streambuf<char>> buffer, std::unique_ptr<SerializeStreamData> data)
        : Stream(std::move(buffer))
        , mData(std::move(data))
    {
    }

    SerializeStream::SerializeStream(SerializeStream &&other)
        : Stream(std::move(other))
        , mData(std::move(other.mData))
    {
    }

    SerializeStream::SerializeStream(SerializeStream &&other, SerializeManager *mgr)
        : Stream(std::move(other))
        , mData(std::move(other.mData))
    {
        //mData->setManager(mgr);
    }

    SerializeStream::~SerializeStream() = default;

    SerializeStream &SerializeStream::operator=(SerializeStream &&other)
    {
        Stream::operator=(std::move(other));
        std::swap(mData, other.mData);
        return *this;
    }

    StreamResult SerializeStream::read(void *buffer, size_t size)
    {
        if (Stream::read(buffer, size) < 0) {
            auto error = STREAM_ERROR(StreamState::OK, *this, true);
            error.mType = streamError(state(), error.mMsg);
            error.mMsg << "after read";
            return error;
        }
        return {};
    }

    StreamResult SerializeStream::readN(std::string &buffer, size_t n)
    {
        STREAM_PROPAGATE_ERROR(skipWs());        

        if (n == 0)
            return {};

        buffer.resize(n, '\0');
        return read(&buffer[0], n);
    }

    StreamResult SerializeStream::readUntil(std::string &buffer, const char *delim)
    {
        STREAM_PROPAGATE_ERROR(skipWs());

        buffer.resize(255);
        size_t i = 0;
        do {
            STREAM_PROPAGATE_ERROR(read(&buffer[i], 1));
            ++i;
        } while (!strchr(delim, buffer[i - 1]));
        buffer.resize(i);

        return {};
    }

    StreamResult SerializeStream::peekN(std::string &buffer, size_t n)
    {
        //assert(!format().mBinary);

        pos_type pos = tell();
        STREAM_PROPAGATE_ERROR(readN(buffer, n));
        seek(pos);
        return {};
    }

    StreamResult SerializeStream::peekUntil(std::string &buffer, const char *c)
    {
        //assert(!format().mBinary);

        pos_type pos = tell();
        STREAM_PROPAGATE_ERROR(readUntil(buffer, c));
        seek(pos);
        return {};
    }

    SerializeManager *SerializeStream::manager() const
    {
        return mData->manager();
    }

    void SerializeStream::setId(ParticipantId id)
    {
        mData->setId(id);
    }

    ParticipantId SerializeStream::id() const
    {
        return mData->id();
    }

    bool SerializeStream::isMaster(AccessMode mode)
    {
        if (mData)
            return mData->isMaster();
        return mode == AccessMode::WRITE;
    }

    SerializeStreamData *SerializeStream::data() const
    {
        return mData.get();
    }

    SerializableUnitList &SerializeStream::serializableList()
    {
        return mData->serializableList();
    }

    SerializableUnitMap &SerializeStream::serializableMap()
    {
        return mData->serializableMap();
    }

    StreamResult SerializeStream::read(SyncableUnitBase *&p)
    {
        UnitId ptr;
        STREAM_PROPAGATE_ERROR(read(ptr));
        assert(ptr <= (std::numeric_limits<UnitId>::max() >> 2));
        if (ptr)
            ptr = (ptr << 2) | static_cast<UnitId>(UnitIdTag::SYNCABLE);
        p = reinterpret_cast<SyncableUnitBase *>(ptr);
        return {};
    }

    StreamResult SerializeStream::read(SerializableDataPtr &p)
    {
        uint32_t ptr;
        STREAM_PROPAGATE_ERROR(read(ptr));
        assert(ptr <= (std::numeric_limits<uint32_t>::max() >> 2));
        if (ptr)
            ptr = (ptr << 2) | static_cast<uint32_t>(UnitIdTag::SERIALIZABLE);
        p.mUnit = reinterpret_cast<void *>(ptr);
        return {};
    }

    StreamResult SerializeStream::operator>>(SyncableUnitBase *&p)
    {
        UnitId ptr;
        STREAM_PROPAGATE_ERROR(operator>>(ptr));
        assert(ptr <= (std::numeric_limits<UnitId>::max() >> 2));
        if (ptr)
            ptr = (ptr << 2) | static_cast<UnitId>(UnitIdTag::SYNCABLE);
        p = reinterpret_cast<SyncableUnitBase *>(ptr);
        return {};
    }

    StreamResult SerializeStream::operator>>(SerializableDataPtr &p)
    {
        uint32_t ptr;
        STREAM_PROPAGATE_ERROR(operator>>(ptr));
        assert(ptr <= (std::numeric_limits<uint32_t>::max() >> 2));
        if (ptr)
            ptr = (ptr << 2) | static_cast<uint32_t>(UnitIdTag::SERIALIZABLE);
        p.mUnit = reinterpret_cast<void *>(ptr);
        return {};
    }

    StreamResult SerializeStream::read(std::string &s)
    {
        uint32_t size;
        STREAM_PROPAGATE_ERROR(read(size));
        s.resize(size);
        STREAM_PROPAGATE_ERROR(read(&s[0], size));
        return {};
    }

    StreamResult SerializeStream::read(ByteBuffer &b)
    {
        uint32_t size;
        STREAM_PROPAGATE_ERROR(read(size));
        std::unique_ptr<std::byte[]> buffer = std::make_unique<std::byte[]>(size);
        STREAM_PROPAGATE_ERROR(read(buffer.get(), size));
        b = ByteBuffer { std::move(buffer), size };
        return {};
    }

    StreamResult SerializeStream::operator>>(ByteBuffer &b)
    {
        std::string base64Encoded;
        STREAM_PROPAGATE_ERROR(operator>>(base64Encoded));
        if (!Base64::decode(b, base64Encoded))
            return STREAM_PARSE_ERROR(*this, false) << "Invalid Base64 String '" << base64Encoded << "'";
        return {};
    }

    StreamResult SerializeStream::read(Void &)
    {
        return {};
    }

    StreamResult SerializeStream::operator>>(Void &)
    {
        return {};
    }

    void SerializeStream::write(const SyncableUnitBase *p)
    {
        Stream::write(SerializeManager::convertPtr(*this, p));
    }

    void SerializeStream::write(SerializableDataConstPtr p)
    {
        uint32_t id = 0;
        if (p) {
            SerializableUnitMap &map = serializableMap();
            auto it = map.try_emplace(p, map.size() + 1).first;
            id = it->second;
        }
        Stream::write(id);
    }

    SerializeStream &SerializeStream::operator<<(const SyncableUnitBase *p)
    {
        Stream::operator<<(SerializeManager::convertPtr(*this, p));
        return *this;
    }
    SerializeStream &SerializeStream::operator<<(SerializableDataConstPtr p)
    {
        uint32_t id = 0;
        if (p) {
            SerializableUnitMap &map = serializableMap();
            auto it = map.try_emplace(p, map.size() + 1).first;
            id = it->second;
        }
        Stream::operator<<(id);
        return *this;
    }

    void SerializeStream::write(const std::string_view &s)
    {
        Stream::write<uint32_t>(s.size());
        Stream::write(s.data(), s.size());
    }

    SerializeStream &SerializeStream::operator<<(const std::string_view &s)
    {
        Stream::operator<<(s);
        return *this;
    }

    void SerializeStream::write(const ByteBuffer &b)
    {
        Stream::write<uint32_t>(b.mSize);
        Stream::write(b.mData, b.mSize);
    }

    SerializeStream &SerializeStream::operator<<(const ByteBuffer &b)
    {
        return operator<<(Base64::encode(b));
    }

    void SerializeStream::write(const Void &)
    {
    }

    SerializeStream &SerializeStream::operator<<(const Void &)
    {
        return *this;
    }

    StreamResult SerializeStream::skipWs(bool overwrite)
    {
        if (!Stream::skipWs(overwrite)) {
            auto error = STREAM_ERROR(StreamState::OK, *this, true);
            error.mType = streamError(state(), error.mMsg);
            error.mMsg << "after skipWs";
        }
        return {};
    }

}
} // namespace Scripting
