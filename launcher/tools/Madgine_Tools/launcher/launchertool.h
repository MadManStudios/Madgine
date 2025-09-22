#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

#include "Modules/ini/inifile.h"

namespace Engine {
namespace Tools {

    struct LauncherTool : Tool<LauncherTool> {

        LauncherTool(ImRoot &root);

        void renderMenu() override;
        bool renderConfiguration(const Filesystem::Path &config) override;
        void loadConfiguration(const Filesystem::Path &config) override;
        void saveConfiguration(const Filesystem::Path &config) override;

        std::string_view key() const override;

    protected:
        Threading::Task<bool> init() override;
        Threading::Task<void> finalize() override;

    private:
        Window::MainWindow *mMainWindow;

        Ini::IniFile mConfiguration;
    };

}
}