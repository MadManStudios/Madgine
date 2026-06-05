#include "../renderlib.h"

#include "shadercache.h"

#include "Generic/guard.h"

#include "Platform/filesystem/fsapi.h"
#include "Platform/process/processapi.h"

#include "Madgine/cli/parameter.h"
#include "Madgine/root/root.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

namespace Engine {
namespace Render {

#ifdef SHADERGEN_LOCATION
    Core::Parameter<Platform::Filesystem::Path> shaderGenPath { { "--shadergen" }, SHADERGEN_LOCATION, "Overwrite ShaderGen location." };
#endif

    Platform::Filesystem::Path ShaderCache::directory()
    {
        return Platform::Filesystem::shippingPath() / "shadercache";
    }

    Threading::Task<bool> ShaderCache::generate(const Platform::Filesystem::Path &path, ShaderObjectPtr object, std::string_view target, ShaderType type)
    {
        bool exists = Platform::Filesystem::exists(path);
#ifdef SHADERGEN_LOCATION

        if (!exists || Platform::Filesystem::fileInfo(path).mLastModified < object->chainTimestamp()) {
            object->generate();

            Platform::Filesystem::Path p = object->metadata().mPath;
            if (p.isRelative()) {
                p = directory() / p;
            }

            std::vector<std::string> commandLine = {
                p,
                path.parentPath().str(),
                "-" + std::string { target },
                "-T",
                type == ShaderType::VertexShader ? "vs_6_2" : "ps_6_2",
                "-E",
                object->entrypoint()
            };

            if (Core::Root::getSingleton().debug()) {
                commandLine.push_back("-g");
            }

            for (const std::string &include : object->metadata().mIncludePaths) {
                commandLine.push_back("-I");
                commandLine.push_back(include);
            }

            auto resultStorage = co_await Platform::Process::runAsync(*shaderGenPath, commandLine, 2s);

            if (!resultStorage.is_value()) {
                if (resultStorage.is_error()) {
                    LOG_ERROR("Running ShaderGen failed: " << std::move(resultStorage).error().mError);
                } else {
                    LOG_ERROR("Running ShaderGen cancelled");
                }
                co_return false;
            }

            auto [result, stdOut, stdErr] = std::move(resultStorage).value();
            if (result != 0) {
                LOG_FATAL("ShaderGen failed with:\n"
                    << stdErr);
                co_return false;
            }
            co_return true;
        }
#endif
        co_return exists;
    }

    std::list<ShaderObjectPtr (*)()> &shaderCacheInternal()
    {
        static std::list<ShaderObjectPtr (*)()> cache;
        return cache;
    }

    void ShaderCache::registerShader(ShaderObjectPtr (*object)())
    {
        shaderCacheInternal().push_back(std::move(object));
    }

    std::list<ShaderObjectPtr> ShaderCache::shaderCache()
    {
        std::list<ShaderObjectPtr> result;
        for (auto f : shaderCacheInternal()) {
            result.push_back(f());
        }
        return result;
    }

}
}