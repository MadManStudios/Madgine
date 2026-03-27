#pragma once

#include "Generic/execution/signal.h"

#include "Interfaces/filesystem/path.h"

#include "Modules/ini/inifile.h"

#include "Madgine/window/layoutloader.h"

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

namespace Engine {

namespace Tools {

    struct MADGINE_CLIENT_TOOLS_EXPORT ProjectManager : Tool<ProjectManager> {
        SERIALIZABLEUNIT(ProjectManager);

        ProjectManager(ImRoot &root);

        virtual Threading::Task<bool> init() override;

        virtual std::string_view key() const override;

#ifndef MADGINE_MAINWINDOW_LAYOUT
        void renderLandingPage();
        void renderConfigurations();
        void renderGameMenu();

        void loadConfiguration(const Filesystem::Path &config) override;
        void saveConfiguration(const Filesystem::Path &config) override;

#endif
        void renderTips();
        void renderSettingsPage();
        void render() override;
        void renderMenu() override;
        bool renderConfiguration(const Filesystem::Path &config) override;
        void renderSettings() override;

        std::vector<Tip> tips() override;

        bool mShowSettings = false;

        bool mShowTipsOnStartup = true;

#ifndef MADGINE_MAINWINDOW_LAYOUT
        void save();
        void load();

        Window::LayoutLoader::Resource *layout() const;
        std::string_view layoutString() const;
        void setLayoutString(std::string_view name);
        void setLayout(Window::LayoutLoader::Resource *layout);

        void setCurrentConfig(const Filesystem::Path &config);

    protected:
        void createProjectDialog();
        void createPluginDialog();

    private:
        Window::LayoutLoader::Resource *mLayout = nullptr;

        bool mShowConfigurations = false;

        std::set<Filesystem::Path> mConfigs;
        Filesystem::Path mCurrentConfig;

        bool mUnsavedConfiguration = false;

        Ini::IniFile mConfiguration;
#endif

    private:
        bool mShowTips = false;
        bool mInitialized = false;

        std::vector<Tip> mTips;
        size_t mTipIndex = 0;

    private:
        Window::MainWindow *mWindow = nullptr;
        Templates *mTemplates = nullptr;
    };

}
}
