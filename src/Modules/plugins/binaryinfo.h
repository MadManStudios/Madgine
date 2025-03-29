#pragma once

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
			const char *mBinaryDir;
			const char *mBinaryFileName;

			const char *mToolsName;

			const char *mPrecompiledHeaderPath;

			const char **mPluginDependencies;
            const char **mPluginGroupDependencies;

			mutable Plugin *mSelf;

		};

#if ENABLE_PLUGINS
		extern "C" const BinaryInfo PLUGIN_LOCAL(binaryInfo);
#define PLUGIN_SELF Engine::Plugins::PLUGIN_LOCAL(binaryInfo).mSelf
#endif

	}
}