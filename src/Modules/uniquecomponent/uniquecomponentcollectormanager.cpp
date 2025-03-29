#include "../moduleslib.h"

#if ENABLE_PLUGINS

#    include "uniquecomponentcollectormanager.h"

#    include "uniquecomponentregistry.h"

#    include "../plugins/pluginmanager.h"

#    include "../plugins/plugin.h"

#include "../plugins/pluginsection.h"

namespace Engine {
namespace UniqueComponent {

    CollectorManager::CollectorManager(Plugins::PluginManager &pluginMgr)
    {
        for (const auto &section : pluginMgr) {
            for (const auto &plugin : section) {
                if (plugin.isLoaded(pluginMgr.selection())) {
                    for (RegistryBase *reg : registryRegistry()) {
                        reg->onPluginLoad(plugin.info());
                    }
                }
            }
        }
    }

}
}

#endif