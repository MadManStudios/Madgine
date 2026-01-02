#include "../renderlib.h"

#include "shadercache.h"

#include "Generic/guard.h"

#include "Interfaces/filesystem/fsapi.h"
#include "Interfaces/process/processapi.h"

#include "Modules/threading/awaitables/awaitablesender.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

namespace Engine {
namespace Render {

    Filesystem::Path ShaderCache::directory()
    {
#if ANDROID
        return "assets:/shadercache";
#else
        return Filesystem::executablePath().parentPath() / "shadercache";
#endif
    }

#if !ANDROID
    static Guard sInitializer {
        []() {
            Filesystem::createDirectories(ShaderCache::directory());
        }
    };
#endif

    Threading::Task<bool> ShaderCache::generate(const Filesystem::Path &path, ShaderObjectPtr object, std::string_view target, ShaderType type)
    {
        bool exists = Filesystem::exists(path);
#ifdef SHADERGEN_LOCATION

        if (!exists || Filesystem::fileInfo(path).mLastModified < object->chainTimestamp()) {
            object->generate();

            Filesystem::Path p = object->metadata().mPath;
            if (p.isRelative()) {
                p = directory() / p;
            }

            std::vector<std::string> commandLine = {
                p,
                path.parentPath().str(),
                "-g",
                std::string { target },
                "-T",
                type == ShaderType::VertexShader ? "vs_6_2" : "ps_6_2",
                "-E",
                object->entrypoint()
            };
            for (const std::string &include : object->metadata().mIncludePaths) {
                commandLine.push_back("-I");
                commandLine.push_back(include);
            }

            auto [result, stdOut, stdErr] = (co_await Process::runAsync(SHADERGEN_LOCATION, commandLine, 2s)).value();
            if (result != 0) {
                LOG_FATAL(stdErr);
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