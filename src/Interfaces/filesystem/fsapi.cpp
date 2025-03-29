#include "../interfaceslib.h"
#include "fsapi.h"

namespace Engine {
namespace Filesystem {

    bool createDirectories(const Path &p)
    {
        if (exists(p))
            return true;

        Path parent = p.parentPath();
        if (!parent.empty())
            if (!createDirectories(parent))
                return false;
        return createDirectory(p);
    }

    bool copyFile(const Path &p, const Path &target)
    {
        Path exactTarget = target;
        if (isDir(exactTarget))
            exactTarget /= p.filename();
        Stream out = openFileWrite(exactTarget, true);
        assert(out);
        Stream in = openFileRead(p);
        assert(in);
        out.pipe(in);
        return true;
    }

    FileQuery listFilesRecursive(const Path &path)
    {
        return { path, true };
    }

    FileQuery listFilesAndDirsRecursive(const Path &path)
    {
        return { path, true, true };
    }

    FileQuery listFiles(const Path &path)
    {
        return { path, false };
    }

    FileQuery listDirs(const Path &path)
    {
        return { path, false, true, false };
    }

    FileQuery listFilesAndDirs(const Path &path)
    {
        return { path, false, true };
    }

    bool isSeparator(char c)
    {
        return c == '/' || c == '\\';
    }

}
}
