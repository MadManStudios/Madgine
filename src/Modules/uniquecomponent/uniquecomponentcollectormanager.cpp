#include "../moduleslib.h"

#if ENABLE_PLUGINS

#    include "../plugins/plugin.h"
#    include "../plugins/pluginmanager.h"
#    include "../plugins/pluginsection.h"
#    include "uniquecomponentcollectormanager.h"
#    include "uniquecomponentregistry.h"

namespace Engine {
namespace Plugins {

    CollectorManager::CollectorManager(Plugins::PluginManager &pluginMgr)
    {
        for (const auto &section : pluginMgr) {
            for (const auto &plugin : section) {
                if (plugin.isLoaded(pluginMgr.selection()) && !plugin.info()->mIsStub) {
                    for (RegistryBase *reg : registryRegistry()) {
                        reg->onPluginLoad(plugin.info());
                    }
                }
            }
        }
        for (RegistryBase *reg : registryRegistry()) {
            reg->init();
        }
    }

}
}

#endif