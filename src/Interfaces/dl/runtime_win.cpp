#include "../interfaceslib.h"

#if WINDOWS

#    include "runtime.h"

#    define NOMINMAX
#    include <Windows.h>
#    include <Psapi.h>

#include "../filesystem/fsapi.h"

namespace Engine {
namespace Dl {

    /*std::set<std::string> listLoadedLibraries()
		{
			std::set<std::string> result;

			DWORD count;
			HMODULE modules[512];
			auto check = EnumProcessModules(GetCurrentProcess(), modules, static_cast<DWORD>(array_size(modules)), &count);
			assert(check);
			count /= sizeof(HMODULE);
			assert(count < array_size(modules));

			for (DWORD i = 0; i < count; ++i)
			{
				char buffer[512];
				auto check2 = GetModuleFileName(modules[i], buffer, sizeof(buffer));
				assert(check2);
				Filesystem::Path path = buffer;
				result.insert(path.stem());
			}

			return result;
		}*/

}
}

#endif