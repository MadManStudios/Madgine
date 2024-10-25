#include "filesystemlib.h"

#include "filemanager.h"

#include "Interfaces/filesystem/path.h"

#include "Interfaces/filesystem/fsapi.h"

#include "Meta/serialize/streams/serializestream.h"

#include "Meta/serialize/streams/formatter.h"

#include "Meta/serialize/streams/serializestreamdata.h"

#include "Meta/serialize/streams/formattedserializestream.h"

namespace Engine {
namespace Filesystem {

    FileManager::FileManager(const std::string &name)
        : SerializeManager(name)
    {
    }

    Serialize::FormattedSerializeStream FileManager::openRead(const Path &path, Serialize::Format format)
    {
        assert(!getSlaveStreamData());

        std::unique_ptr<Serialize::Formatter> formatter = format();

        Stream stream = openFileRead(path, formatter->mBinary);
        if (stream) {
            return Serialize::FormattedSerializeStream {
                std::move(formatter),
                wrapStream(std::move(stream), true)
            };
        } else {
            return {};
        }
    }

    Serialize::FormattedSerializeStream FileManager::openWrite(const Path &path, Serialize::Format format)
    {
        std::unique_ptr<Serialize::Formatter> formatter = format();

        Stream stream = openFileWrite(path, formatter->mBinary);
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
