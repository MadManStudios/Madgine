#pragma once

#if ENABLE_PLUGINS

namespace Engine {
namespace Plugins {

    struct MODULES_EXPORT CollectorManager {
        CollectorManager(Plugins::PluginManager &pluginMgr);
    };

}
}

#endif