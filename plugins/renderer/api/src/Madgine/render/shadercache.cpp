#include "../renderlib.h"

#include "shadercache.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Interfaces/filesystem/fsapi.h"

#include "Interfaces/process/processapi.h"

#include "Modules/threading/awaitables/awaitablesender.h"

#include "Generic/guard.h"

namespace Engine {
namespace Render {

    Filesystem::Path ShaderCache::directory()
    {
        return ".shadercache";
    }

    static Guard sInitializer {
        []() {
            Filesystem::createDirectories(ShaderCache::directory());
        }
    };

    Threading::Task<void> ShaderCache::generate(const Filesystem::Path &path, ShaderObjectPtr object, std::string_view target, std::string_view profile)
    {

        std::chrono::file_clock::time_point time = std::chrono::file_clock::time_point::min();

        if (Filesystem::exists(path)) {
            time = Filesystem::fileInfo(path).mLastModified;
        }

        if (time < object->chainTimestamp()) {
            object->generate();

            std::vector<std::string> commandLine = {
                object->metadata().mPath,                
                path.parentPath().str(),
                "-g",
                std::string { target },
                "-T",
                std::string { profile },
                "-E",
                object->entrypoint()
            };
            for (const std::string &include : object->metadata().mIncludePaths) {
                commandLine.push_back("-I");
                commandLine.push_back(include);
            }
            auto [result, stdOut, stdErr] = (co_await Process::runAsync("M:\\MadManStudios\\MadgineShadergen\\out\\build\\x64-debug\\ShaderGen.exe", commandLine, 2s)).value();
            if (result != 0) {
                LOG_FATAL(stdErr);
            }
        }
    }

}
}