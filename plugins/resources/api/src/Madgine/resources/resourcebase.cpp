#include "../resourceslib.h"

#include "resourcebase.h"

#include "Generic/execution/algorithm.h"
#include "Generic/execution/execution.h"

#include "Interfaces/fetch/fetchapi.h"
#include "Interfaces/filesystem/async.h"
#include "Interfaces/filesystem/fsapi.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "resourcemanager.h"

METATABLE_BEGIN(Engine::Resources::ResourceBase)
    READONLY_PROPERTY(Name, name)
METATABLE_END(Engine::Resources::ResourceBase)

namespace Engine {
namespace Resources {
    ResourceBase::ResourceBase(const std::string &name, Filesystem::Path path)
        : mName(name)
        , mPath(std::move(path))
    {
    }

    void ResourceBase::setPath(const Filesystem::Path &path)
    {
        mPath = path;
    }

    const Filesystem::Path &ResourceBase::path() const
    {
        return mPath;
    }

    std::string_view ResourceBase::extension()
    {
        return mPath.extension();
    }

    std::string_view ResourceBase::name()
    {
        return mName;
    }

    Stream ResourceBase::readAsStream(bool isBinary) const
    {
        return Filesystem::openFileRead(mPath, isBinary);
    }

    std::string ResourceBase::readAsText() const
    {
        Stream buffer = readAsStream();
        return std::string { buffer.iterator(), buffer.end() };
    }

    std::vector<unsigned char> ResourceBase::readAsBlob() const
    {
        Stream buffer = readAsStream(true);
        return std::vector<unsigned char> { buffer.iterator(), buffer.end() };
    }

    Execution::Sender<GenericResult, ByteBuffer> ResourceBase::readAsync() const
    {
        std::string_view protocol = mPath.protocol();

        if (protocol == "http" || protocol == "https" || protocol == "ftp") {
            co_return co_await FetchSender<std::vector<std::byte>>(mPath, { mPath });
        } else if (!protocol.empty()) {
            LOG_WARNING("Unrecognized file protocol '" << protocol << "'");
        }

        co_return co_await Filesystem::readFileAsync(mPath);
    }

    std::string ResourceBase::plugin() const
    {
        return Resources::ResourceManager::getSingleton().makeRelative(mPath).first;
    }

}
}
