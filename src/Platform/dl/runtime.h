#pragma once

#include "../filesystem/filequery.h"

namespace Engine {
namespace Platform {
    namespace Dl {

        PLATFORM_EXPORT Filesystem::FileQuery listSharedLibraries();
        // PLATFORM_EXPORT std::set<std::string> listLoadedLibraries();
    }
}
}