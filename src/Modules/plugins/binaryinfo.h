#pragma once

#include "Interfaces/filesystem/path.h"

#include "pluginlocal.h"

namespace Engine {
namespace Plugins {

    struct BinaryInfo {

        size_t mMajorVersion;
        size_t mMinorVersion;
        size_t mPatchNumber;

        const char *mName;

        const char *mProjectRoot;
        const char *mSourceRoot;
        const char *mBinaryFileName;

        const char *mToolsName;

        const char *mPrecompiledHeaderPath;

        const char **mPluginDependencies;
        const char **mPluginGroupDependencies;

        bool mIsStub;
        Filesystem::Path mDataPath;
    };

#if ENABLE_PLUGINS
    extern "C" const BinaryInfo PLUGIN_LOCAL(binaryInfo);
#endif

}
}