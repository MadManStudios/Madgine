#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

#include "Modules/ini/inifile.h"

namespace Engine {
namespace Tools {

    struct LauncherTool : Tool<LauncherTool> {

        LauncherTool(ImRoot &root);

        virtual void render() override;
        virtual bool renderConfiguration(const Filesystem::Path &config) override;
        virtual void loadConfiguration(const Filesystem::Path &config) override;
        virtual void saveConfiguration(const Filesystem::Path &config) override;

        std::string_view key() const override;

    protected:
        virtual Threading::Task<bool> init() override;
        virtual Threading::Task<void> finalize() override;

    private:
        Window::MainWindow *mMainWindow;

        Ini::IniFile mConfiguration;
    };

}
}

REGISTER_TYPE(Engine::Tools::LauncherTool)