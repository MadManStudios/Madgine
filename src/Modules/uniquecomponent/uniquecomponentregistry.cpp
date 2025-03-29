#include "../moduleslib.h"

#if ENABLE_PLUGINS

#    include "uniquecomponentregistry.h"

namespace Engine {
namespace UniqueComponent {

    MODULES_EXPORT std::vector<RegistryBase *> &
    registryRegistry()
    {
        static std::vector<RegistryBase *> dummy;
        return dummy;
    }

}
} // namespace Engine

#endif