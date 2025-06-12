#include "../renderlib.h"

#include "shaderobject.h"

#include "Interfaces/filesystem/path.h"

#include "Interfaces/filesystem/fsapi.h"

#include "shadercache.h"

namespace Engine {
namespace Render {

    const ShaderObjectBase *ShaderObjectPtr::operator->() const
    {
        return mObject.get();
    }

    std::string ShaderObjectBase::name() const
    {
        return std::string { metadata().mPath.stem() } + ":" + entrypoint();
    }

    std::ostream &operator<<(std::ostream &out, const ShaderObjectPtr &p)
    {
        out << p.mObject->name();
        return out;
    }

    constexpr ShaderObjectPtr::operator bool() const
    {
        return static_cast<bool>(mObject);
    }

    ShaderObjectBase::ShaderObjectBase(std::vector<ShaderObjectPtr> dependencies)
        : mDependencies(std::move(dependencies))
    {
    }

    void ShaderObjectBase::generate() const
    {
        Filesystem::Path p = metadata().mPath;

        if (p.isRelative()) {
            p = ShaderCache::directory() / p;
        }

        bool needsToRegenerate = !Filesystem::exists(p);

        std::chrono::file_clock::time_point time = needsToRegenerate ? std::chrono::file_clock::time_point::min() : timestamp();

        for (const ShaderObjectPtr &dep : mDependencies) {
            dep->generate();
            needsToRegenerate |= time < dep->timestamp();
        }

        if (needsToRegenerate) {
            std::ofstream of { p };
            toHLSL(of);
        }
    }

    std::chrono::file_clock::time_point ShaderObjectBase::timestamp() const
    {
        Filesystem::Path p = metadata().mPath;

        if (p.isRelative()) {
            p = ShaderCache::directory() / p;
        }

        assert(Filesystem::exists(p));

        return Filesystem::fileInfo(p).mLastModified;
    }

    std::chrono::file_clock::time_point ShaderObjectBase::chainTimestamp() const
    {
        std::chrono::file_clock::time_point acc = timestamp();

        for (const ShaderObjectPtr &dep : mDependencies) {
            acc = std::max(dep->chainTimestamp(), acc);
        }

        return acc;
    }

}
}