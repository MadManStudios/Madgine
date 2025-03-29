#include "../interfaceslib.h"

#if LINUX

#    include "fsapi.h"

#    include <dirent.h>
#    include <sys/stat.h>
#    include <unistd.h>
#    include <wordexp.h>

namespace Engine {
namespace Filesystem {

    void setup(void *)
    {
    }

    Path executablePath()
    {
        char buffer[512];

        auto result = readlink("/proc/self/exe", buffer, sizeof(buffer));
        assert(result > 0);
        buffer[result] = '\0';

        return Path(buffer).parentPath();
    }

    std::string executableName()
    {
        //TODO
        char buffer[512];

        auto result = readlink("/proc/self/exe", buffer, sizeof(buffer));
        assert(result > 0);
        buffer[result] = '\0';

        return std::string { Path(buffer).stem() };
    }

    Path appDataPath()
    {
        Path result { "~/." + executableName() };

        if (!exists(result))
            createDirectory(result);

        return result;
    }

    bool isDir(const Path &p)
    {
        struct stat statbuffer;
        auto result = stat(p.c_str(), &statbuffer);
        assert(result != -1);
        return (statbuffer.st_mode & S_IFMT) == S_IFDIR;
    }

    bool createDirectory(const Path &p)
    {
        std::string s = p.str();
        makeNormalized(s);
        auto result = mkdir(s.c_str(), 0700);
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
        if (p.empty())
            return;

        wordexp_t word;

        wordexp(p.c_str(), &word, 0);
        assert(word.we_wordc == 1);
        p = word.we_wordv[0];
        wordfree(&word);
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
        std::string s = p.str();
        makeNormalized(s);
        if (buffer->open(s.c_str(), static_cast<std::ios_base::openmode>(std::ios_base::in | (isBinary ? std::ios_base::binary : 0))))
            return { std::move(buffer) };
        else
            return {};
    }

    Stream openFileWrite(const Path &p, bool isBinary)
    {
        std::unique_ptr<std::filebuf> buffer = std::make_unique<std::filebuf>();
        std::string s = p.str();
        makeNormalized(s);
        if (buffer->open(s.c_str(), static_cast<std::ios_base::openmode>(std::ios_base::out | (isBinary ? std::ios_base::binary : 0))))
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