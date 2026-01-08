#include "../renderlib.h"

#include "shaderobject.h"

#include "Interfaces/filesystem/fsapi.h"
#include "Interfaces/filesystem/path.h"

#include "shadercache.h"

namespace Engine {
namespace Render {

    const ShaderObjectBase *ShaderObjectPtr::operator->() const
    {
        return mObject.get();
    }

    ShaderObjectPtr::operator bool() const
    {
        return static_cast<bool>(mObject);
    }

    std::string ShaderObjectBase::name() const
    {
        return std::string { metadata().mPath.stem() } + ":" + entrypoint();
    }

    std::ostream &operator<<(std::ostream &out, const ShaderObjectPtr &p)
    {
        if (p.mObject)
            out << p.mObject->name();
        else
            out << "<null>";
        return out;
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
            if (!of.is_open()) {
                LOG_ERROR("Failed to open shader file for writing: " << p);
                if (!Filesystem::exists(ShaderCache::directory())) {
                    LOG_ERROR("note: shadercache folder missing");
                }
            }
            toHLSL(of);
        }
    }

    std::chrono::file_clock::time_point ShaderObjectBase::timestamp() const
    {
        Filesystem::Path p = metadata().mPath;

        if (p.isRelative()) {
            p = ShaderCache::directory() / p;
        }

        return Filesystem::exists(p) ? Filesystem::fileInfo(p).mLastModified : std::chrono::file_clock::time_point::min();
    }

    std::chrono::file_clock::time_point ShaderObjectBase::chainTimestamp() const
    {
        std::chrono::file_clock::time_point acc = timestamp();

        for (const ShaderObjectPtr &dep : mDependencies) {
            acc = std::max(dep->chainTimestamp(), acc);
        }

        return acc;
    }

    MergedShaderObjectBase::MergedShaderObjectBase(const ShaderObjectPtr &first, const ShaderObjectPtr &second)
        : ShaderObjectBase({ first, second })
    {
        mMetadata.mPath = first->entrypoint() + "_" + second->entrypoint() + ".hlsl";
        std::ranges::set_union(first->metadata().mIncludePaths, second->metadata().mIncludePaths, std::back_inserter(mMetadata.mIncludePaths));
    }

    std::string MergedShaderObjectBase::entrypoint() const
    {
        return std::string { mMetadata.mPath.stem() };
    }

    const ShaderMetadata &MergedShaderObjectBase::metadata() const
    {
        return mMetadata;
    }

    void MergedShaderObjectBase::toHLSLImpl(std::ostream &o, std::string_view r, std::string_view in) const
    {
        for (const ShaderObjectPtr &dependency : mDependencies) {
            o << "#include \"" << dependency->metadata().mPath << "\"\n";
        }
        o << "\n";
        o << r << " " << entrypoint() << "(" << in << " IN) {\n"
          << "    return " << mDependencies[1]->entrypoint() << "(" << mDependencies[0]->entrypoint() << "(IN));\n"
          << "}";
    }

}
}