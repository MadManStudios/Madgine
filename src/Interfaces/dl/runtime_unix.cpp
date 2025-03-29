#include "../interfaceslib.h"

#if UNIX

#    include "runtime.h"
#    include <dirent.h>

#    include <unistd.h>

#include "../filesystem/fsapi.h"

#if LINUX
#    include <link.h>
#endif

namespace Engine {
namespace Dl {



    /*static int visitModule(struct dl_phdr_info *info, size_t size, void *data) {
			if (info->dlpi_name)
			{
				Filesystem::Path file = Filesystem::Path(info->dlpi_name).filename();
				if (file.extension() == SHARED_LIB_SUFFIX)
				{
					std::string name = file.stem();
					if (StringUtil::startsWith(name, SHARED_LIB_PREFIX))
					{
						std::set<std::string> *modules = static_cast<std::set<std::string> *>(data);
						modules->insert(name.substr(strlen(SHARED_LIB_PREFIX)));
					}
				}
			}
			return 0;
		}

		std::set<std::string> listLoadedLibraries()
		{
			std::set<std::string> result;
			dl_iterate_phdr(visitModule, &result);
			return result;
		}*/

}
}

#endif
