#pragma once

#include "Meta/serialize/serializemanager.h"
#include "Generic/bytebuffer.h"

namespace Engine {
namespace Memory {

    
    struct MADGINE_MEMORY_SERIALIZE_EXPORT MemoryManager : Serialize::SerializeManager {
        MemoryManager(const std::string &name);
        MemoryManager(const MemoryManager &) = delete;
        MemoryManager(MemoryManager &&) noexcept;
        virtual ~MemoryManager();

        void operator=(const MemoryManager &) = delete;

        Serialize::FormattedSerializeStream openRead(ByteBuffer buffer, Serialize::Format format);
        Serialize::FormattedSerializeStream openWrite(WritableByteBuffer buffer, Serialize::Format format);

    };
}
}
