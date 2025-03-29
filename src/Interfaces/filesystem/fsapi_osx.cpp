#include "../interfaceslib.h"

#if OSX

#    include "fsapi.h"

#    include <dirent.h>
#    include <sys/stat.h>
#    include <unistd.h>

#include <mach-o/dyld.h>

namespace Engine {
namespace Filesystem {

void setup(void *)
{
    
}

Path executablePath()
{
    char buffer[512];

    uint32_t size = 512;

    auto result = _NSGetExecutablePath(buffer, &size);
    assert(result == 0);
    
    return Path(buffer).parentPath();
}

    std::string executableName()
    {
        char buffer[512];

        uint32_t size = 512;

        auto result = _NSGetExecutablePath(buffer, &size);
        assert(result == 0);
        
        return std::string{Path(buffer).stem()};
    }

Path appDataPath()
{
    Path result = Path{getenv("HOME")} / ("." + executableName());

    if (!exists(result))
        createDirectory(result);

    return result;
}

    bool isDir(const Path& p) {
        struct stat statbuffer;
        auto result = stat(p.c_str(), &statbuffer);
        assert(result != -1);
        return (statbuffer.st_mode & S_IFMT) == S_IFDIR;
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
