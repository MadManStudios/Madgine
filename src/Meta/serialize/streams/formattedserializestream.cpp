#include "../../metalib.h"

#include "formattedserializestream.h"

#include "comparestreamid.h"

namespace Engine {
namespace Serialize {

    bool CompareStreamId::operator()(const FormattedSerializeStream &first, const FormattedSerializeStream &second) const
    {
        return first.id() < second.id();
    }

    bool CompareStreamId::operator()(const FormattedSerializeStream &first, ParticipantId second) const
    {
        return first.id() < second;
    }

    bool CompareStreamId::operator()(ParticipantId first, const FormattedSerializeStream &second) const
    {
        return first < second.id();
    }

    FormattedSerializeStream::FormattedSerializeStream(std::unique_ptr<Serialize::Formatter> format, SerializeStream stream)
        : mFormatter(std::move(format))
    {
        mFormatter->setupStream(std::move(stream));
    }

    void FormattedSerializeStream::beginExtendedWrite(const char *name, size_t count)
    {
        mFormatter->beginExtendedWrite(name, count);
    }

    void FormattedSerializeStream::beginCompoundWrite(const char *name)
    {
        mFormatter->beginCompoundWrite(name);
    }
    void FormattedSerializeStream::endCompoundWrite(const char *name)
    {
        mFormatter->endCompoundWrite(name);
    }

    StreamResult FormattedSerializeStream::beginExtendedRead(const char *name, size_t count)
    {
        return mFormatter->beginExtendedRead(name, count);
    }

    StreamResult FormattedSerializeStream::beginCompoundRead(const char *name)
    {
        return mFormatter->beginCompoundRead(name);
    }
    StreamResult FormattedSerializeStream::endCompoundRead(const char *name)
    {
        return mFormatter->endCompoundRead(name);
    }

    void FormattedSerializeStream::beginContainerWrite(const char *name, uint32_t size)
    {
        mFormatter->beginContainerWrite(name, size);
    }
    void FormattedSerializeStream::endContainerWrite(const char *name)
    {
        mFormatter->endContainerWrite(name);
    }

    StreamResult FormattedSerializeStream::beginContainerRead(const char *name, bool sized)
    {
        return mFormatter->beginContainerRead(name, sized);
    }
    StreamResult FormattedSerializeStream::endContainerRead(const char *name)
    {
        return mFormatter->endContainerRead(name);
    }

    bool FormattedSerializeStream::hasContainerItem()
    {
        return mFormatter->hasContainerItem();
    }

    StreamResult FormattedSerializeStream::lookupFieldName(std::string &name)
    {
        return mFormatter->lookupFieldName(name);
    }

    bool FormattedSerializeStream::supportsNameLookup() const
    {
        return mFormatter->mSupportNameLookup;
    }

    FormattedSerializeStream::operator bool() const
    {
        return static_cast<bool>(mFormatter);
    }

    SerializeStreamData *FormattedSerializeStream::data() const
    {
        return mFormatter->stream().data();
    }

    bool FormattedSerializeStream::isMaster(AccessMode mode)
    {
        return mFormatter->stream().isMaster(mode);
    }

    bool FormattedSerializeStream::isBinary()
    {
        return mFormatter->mBinary;
    }

    void FormattedSerializeStream::setId(ParticipantId id)
    {
        return mFormatter->stream().setId(id);
    }

    ParticipantId FormattedSerializeStream::id() const
    {
        return mFormatter->stream().id();
    }

    SerializeManager *FormattedSerializeStream::manager()
    {
        return mFormatter->stream().manager();
    }

    SerializableUnitList &FormattedSerializeStream::serializableList()
    {
        return mFormatter->stream().serializableList();
    }

    SerializeStream &FormattedSerializeStream::stream()
    {
        return mFormatter->stream();
    }

}
}