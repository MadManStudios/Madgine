#pragma once

#include "Modules/plugins/inifile.h"

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

namespace Engine {
namespace Tools {

    struct LauncherTool : Tool<LauncherTool> {

        LauncherTool(ImRoot &root);

        void renderMenu() override;
        bool renderConfiguration(const Platform::Filesystem::Path &config) override;
        void loadConfiguration(const Platform::Filesystem::Path &config) override;
        void saveConfiguration(const Platform::Filesystem::Path &config) override;

        std::string_view key() const override;

    protected:
        Threading::Task<bool> init() override;
        Threading::Task<void> finalize() override;

    private:
        Core::MainWindow *mMainWindow;

        Plugins::IniFile mConfiguration;
    };

}
}