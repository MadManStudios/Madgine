#include "filesystemlib.h"

#include "filemanager.h"

#include "Platform/filesystem/fsapi.h"
#include "Platform/filesystem/path.h"

#include "Meta/serialize/streams/formattedserializestream.h"
#include "Meta/serialize/streams/formatter.h"
#include "Meta/serialize/streams/serializestream.h"
#include "Meta/serialize/streams/serializestreamdata.h"

namespace Engine {
namespace Serialize {

    FileManager::FileManager(const std::string &name)
        : SerializeManager(name)
    {
    }

    Serialize::FormattedSerializeStream FileManager::openRead(const Platform::Filesystem::Path &path, Format format)
    {
        assert(!getSlaveStreamData());

        std::unique_ptr<Serialize::Formatter> formatter = format();

        Stream stream = Platform::Filesystem::openFileRead(path, formatter->mBinary);
        if (stream) {
            return Serialize::FormattedSerializeStream {
                std::move(formatter),
                wrapStream(std::move(stream), true)
            };
        } else {
            return {};
        }
    }

    Serialize::FormattedSerializeStream FileManager::openWrite(const Platform::Filesystem::Path &path, Format format)
    {
        std::unique_ptr<Serialize::Formatter> formatter = format();

        Stream stream = Platform::Filesystem::openFileWrite(path, formatter->mBinary);
        if (stream) {
            return Serialize::FormattedSerializeStream {
                std::move(formatter),
                wrapStream(std::move(stream))
            };
        } else {
            return {};
        }
    }

}
}
