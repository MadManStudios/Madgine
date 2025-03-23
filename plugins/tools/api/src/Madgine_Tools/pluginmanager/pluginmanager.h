#pragma once

#if ENABLE_PLUGINS

#    include "../toolbase.h"
#    include "../toolscollector.h"

#include "Modules/ini/inifile.h"

namespace Engine {
namespace Tools {

    struct PluginManager : Tool<PluginManager> {
        SERIALIZABLEUNIT(PluginManager)

        PluginManager(ImRoot &root);

        virtual void render() override;
        virtual bool renderConfiguration(const Filesystem::Path &config) override;
        virtual void loadConfiguration(const Filesystem::Path &config) override;
        virtual void saveConfiguration(const Filesystem::Path &config) override;

        bool renderPluginSelection(bool isConfiguration);

        virtual Threading::Task<bool> init() override;

        std::string_view key() const override;

    private:
        Plugins::PluginManager &mManager;
        Ini::IniFile mCurrentConfiguration;
    };

}
}

REGISTER_TYPE(Engine::Tools::PluginManager)

#endif