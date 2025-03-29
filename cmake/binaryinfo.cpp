#include "Modules/moduleslib.h"

#include "Modules/plugins/binaryinfo.h"

/*namespace Engine{
	namespace Plugins {
	*/

static const char *dependencies[] = { PLUGIN_DEPENDENCIES };
static const char *groupDependencies[] = { PLUGIN_GROUP_DEPENDENCIES };

extern "C" DLL_EXPORT const Engine::Plugins::BinaryInfo PLUGIN_LOCAL(binaryInfo) {

    BINARY_MAJOR_VERSION,
    BINARY_MINOR_VERSION,
    BINARY_PATCH_NUMBER,

    STRINGIFY2(PROJECT_NAME),

    BINARY_PROJECT_ROOT,
    BINARY_SOURCE_ROOT,
    BINARY_BINARY_DIR,
    BINARY_FILENAME,

    TOOLS_NAME,

    PRECOMPILED_HEADER_PATH,
    dependencies,
    groupDependencies

};

/*
	}
}*/
