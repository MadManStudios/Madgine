#pragma once

#include "Generic/stream.h"

#include "streamresult.h"

#include "../hierarchy/serializableunitptr.h"

namespace Engine {
namespace Serialize {

    struct META_EXPORT SerializeStream : Stream {
    public:
        SerializeStream();
        SerializeStream(std::unique_ptr<std::basic_streambuf<char>> buffer, std::unique_ptr<SerializeStreamData> data = {});
        SerializeStream(SerializeStream &&other);
        SerializeStream(SerializeStream &&other, SerializeManager *mgr);
        ~SerializeStream();

        SerializeStream &operator=(SerializeStream &&other);

        SerializeStreamData *data() const;
        SerializeManager *manager() const;
        void setId(ParticipantId id);
        ParticipantId id() const;
        bool isMaster(AccessMode mode);
        SerializableUnitList &serializableList();
        SerializableUnitMap &serializableMap();

        StreamResult readN(std::string &buffer, size_t n);
        StreamResult peekN(std::string &buffer, size_t n);
        StreamResult readUntil(std::string &buffer, const char *delim);
        StreamResult peekUntil(std::string &buffer, const char *delim);

        StreamResult read(void *buffer, size_t size);
        template <typename T>
        StreamResult read(T &t)
        {
            return read(&t, sizeof(T));
        }
        template <typename T>
        StreamResult operator>>(T &t)
        {
            Stream::operator>>(t);
            if (!*this)
                return STREAM_PARSE_ERROR(*this, false) << "Expected: <" << typeid(T).name() << ">";
            return {};
        }

        template <typename T>
        StreamResult read(T *&p)
        {
            if constexpr (std::convertible_to<T *, SyncableUnitBase *>) {
                SyncableUnitBase *unit;
                STREAM_PROPAGATE_ERROR(read(unit));
                p = reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(unit));
            } else {
                SerializableDataPtr unit;
                STREAM_PROPAGATE_ERROR(read(unit));
                p = reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(unit.mUnit));
            }
            return {};
        }

        template <typename T>
        StreamResult operator>>(T *&p)
        {
            if constexpr (std::convertible_to<T *, SyncableUnitBase *>) {
                SyncableUnitBase *unit;
                STREAM_PROPAGATE_ERROR(operator>>(unit));
                p = reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(unit));
            } else {
                SerializableDataPtr unit;
                STREAM_PROPAGATE_ERROR(operator>>(unit));
                p = reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(unit.mUnit));
            }
            return {};
        }

        StreamResult read(SyncableUnitBase *&p);
        StreamResult read(SerializableDataPtr &p);

        StreamResult operator>>(SyncableUnitBase *&p);
        StreamResult operator>>(SerializableDataPtr &p);

        StreamResult read(std::string &s);
        StreamResult read(String auto &s)
        {
            std::string string;
            STREAM_PROPAGATE_ERROR(read(string));
            s = std::move(string);
            return {};
        }

        StreamResult operator>>(String auto &s) requires(!std::same_as<decltype(s), std::string &>)
        {
            std::string string;
            STREAM_PROPAGATE_ERROR(operator>>(string));
            s = std::move(string);
            return {};
        }

        StreamResult read(ByteBuffer &b);

        StreamResult operator>>(ByteBuffer &b);

        StreamResult read(Void &);

        StreamResult operator>>(Void &);

        template <typename T>
        requires(!Pointer<T> && !StringViewable<T>) void write(const T &t)
        {
            Stream::write(t);
        }

        template <typename T>
        requires(!Pointer<T> && !StringViewable<T>) SerializeStream &operator<<(const T &t)
        {
            Stream::operator<<(t);
            return *this;
        }

        void write(const SyncableUnitBase *p);
        void write(SerializableDataConstPtr);

        SerializeStream &operator<<(const SyncableUnitBase *);
        SerializeStream &operator<<(SerializableDataConstPtr);

        void write(const std::string_view &s);
        void write(const StringViewable auto &s)
        {
            write(std::string_view { s });
        }

        SerializeStream &operator<<(const std::string_view &);
        SerializeStream &operator<<(const StringViewable auto &s)
        {
            return operator<<(std::string_view { s });
        }

        void write(const ByteBuffer &b);

        SerializeStream &operator<<(const ByteBuffer &b);

        void write(const Void &);

        SerializeStream &operator<<(const Void &);

        StreamResult skipWs(bool overwrite = false);

    protected:
        std::unique_ptr<SerializeStreamData> mData;
    };
}
}
