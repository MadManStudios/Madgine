#include "../interfaceslib.h"

#if EMSCRIPTEN

#    include "fsapi.h"

#    include <dirent.h>
#    include <sys/stat.h>
#    include <unistd.h>
#    include <emscripten.h>

#    include <emscripten/html5.h>


namespace Engine {
namespace Filesystem {    

    void (*sSetupSuccessFunction)() = nullptr;

    void sync(void *)
    {
        EM_ASM(
            FS.syncfs(
                false, function(err) {
                    assert(!err);
                }););
    }

    EMSCRIPTEN_KEEPALIVE DLL_EXPORT_TAG extern "C" void setupDoneImpl() {
        sSetupSuccessFunction();
        emscripten_set_interval(&sync, 15000, nullptr);
    }

    DLL_EXPORT void setup(void *fn)
    {
        sSetupSuccessFunction = *static_cast<void (**)()>(fn);

        EM_ASM(
            FS.mkdir('/cwd');
            FS.mount(IDBFS, {}, '/cwd');

            FS.syncfs(
                true, function(err) {
                    assert(!err);
                    _setupDoneImpl();
                }););
        setCwd("/cwd");
    }

    Path executablePath()
    {
        return Path { "/cwd" };
    }

    std::string executableName()
    {
        return "MadgineExecutable";
    }

    Path appDataPath()
    {
        return Path { "/cwd" };
    }

    bool isDir(const Path &p)
    {
        struct stat statbuffer;
        auto result = stat(p.c_str(), &statbuffer);
        assert(result != -1);
        return (statbuffer.st_mode & S_IFMT) == S_IFDIR;
    }

    void setCwd(const Path &p)
    {
        auto result = chdir(p.c_str());
        assert(result == 0);
    }

    Path getCwd()
    {
        char buffer[256];
        auto result = getcwd(buffer, sizeof(buffer));
        assert(result);
        return buffer;
    }

    bool createDirectory(const Path &p)
    {
        auto result = mkdir(p.c_str(), 0700);
        return result == 0;
    }

    bool exists(const Path &p)
    {
        return access(p.c_str(), F_OK) != -1;
    }

    bool remove(const Path &p)
    {
        return ::remove(p.c_str());
    }

    void makeNormalized(std::string &p)
    {
    }

    bool isAbsolute(const Path &p)
    {
        return p.c_str()[0] == '/';
    }

    bool isEqual(const Path &p1, const Path &p2)
    {
        return p1 == p2;
    }

    Stream openFileRead(const Path &p, bool isBinary)
    {
        std::unique_ptr<std::filebuf> buffer = std::make_unique<std::filebuf>();
        if (buffer->open(p.c_str(), static_cast<std::ios_base::openmode>(std::ios_base::in | (isBinary ? std::ios_base::binary : 0))))
            return { std::move(buffer) };
        else
            return {};
    }

    Stream openFileWrite(const Path &p, bool isBinary)
    {
        std::unique_ptr<std::filebuf> buffer = std::make_unique<std::filebuf>();
        if (buffer->open(p.c_str(), static_cast<std::ios_base::openmode>(std::ios_base::out | (isBinary ? std::ios_base::binary : 0))))
            return { std::move(buffer) };
        else
            return {};
    }

    bool isValidPath(const std::string &p)
    {
        for (char c : p)
            if ((!std::isalnum(c) && !isSeparator(c) && !std::ispunct(c) && c != ' ')
                || c == '<'
                || c == '>'
                || c == ':'
                || c == '"'
                || c == '|'
                || c == '?'
                || c == '*')
                return false;
        return true;
    }

    FileInfo fileInfo(const Path &path)
    {
        struct stat stats;
        stat(path.c_str(), &stats);
        return {
            static_cast<size_t>(stats.st_size)
        };
    }

}
}

#endif