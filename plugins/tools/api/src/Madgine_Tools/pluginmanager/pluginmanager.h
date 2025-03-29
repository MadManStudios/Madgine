#pragma once

#if ENABLE_PLUGINS

#    include "../toolbase.h"
#    include "../toolscollector.h"

#include "Modules/ini/inifile.h"

#include "Generic/execution/lifetime.h"

#include "Madgine/curl/curl.h"

namespace Engine {
namespace Tools {

    struct PluginManager : Tool<PluginManager> {
        SERIALIZABLEUNIT(PluginManager)

        PluginManager(ImRoot &root);

        void render() override;
        bool renderConfiguration(const Filesystem::Path &config) override;
        void loadConfiguration(const Filesystem::Path &config) override;
        void saveConfiguration(const Filesystem::Path &config) override;

        bool renderPluginSelection(bool isConfiguration);

        Threading::Task<bool> init() override;
        Threading::Task<void> finalize() override;

        void update() override;

        std::string_view key() const override;

    private:
        Plugins::PluginManager &mManager;
        Ini::IniFile mCurrentConfiguration;

        struct PluginSource {
            Filesystem::Path mIcon;
            std::string mName;
        };
        std::vector<PluginSource> mSources;
        struct Icon {
            bool mFetching = false;
            std::string mContent;
        };
        std::map<std::string, Icon> mIconCache;

        Execution::Lifetime<> mLifetime;
        CurlManager mCurl;
    };

}
}

REGISTER_TYPE(Engine::Tools::PluginManager)

#endif