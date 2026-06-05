#include "memorylib.h"

#include "memorymanager.h"

#include "Meta/serialize/streams/formattedserializestream.h"
#include "Meta/serialize/streams/formatter.h"
#include "Meta/serialize/streams/serializestream.h"
#include "Meta/serialize/streams/serializestreamdata.h"

#include "memorybuffer.h"

namespace Engine {
namespace Serialize {

    MemoryManager::MemoryManager(const std::string &name)
        : SerializeManager(name)
    {
    }

    MemoryManager::MemoryManager(MemoryManager &&other) noexcept
        : SerializeManager(std::forward<MemoryManager>(other))
    {
    }

    MemoryManager::~MemoryManager()
    {
    }

    Serialize::FormattedSerializeStream MemoryManager::openRead(Memory::ByteBuffer buffer, Serialize::Format format)
    {
        Serialize::FormattedSerializeStream stream { format(), wrapStream(Stream { std::make_unique<MemoryReadBuffer>(std::move(buffer)) }, true) };
        return stream;
    }

    Serialize::FormattedSerializeStream MemoryManager::openWrite(Memory::WritableByteBuffer &buffer, Serialize::Format format)
    {
        return { format(), wrapStream(Stream { std::make_unique<MemoryWriteBuffer>(buffer) }) };
    }

}
}
