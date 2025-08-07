#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

#include "Interfaces/filesystem/path.h"

#include "Generic/execution/signal.h"

#include "Modules/ini/inifile.h"

namespace Engine {

namespace Tools {

    struct MADGINE_CLIENT_TOOLS_EXPORT ProjectManager : Tool<ProjectManager> {
        SERIALIZABLEUNIT(ProjectManager);

        ProjectManager(ImRoot &root);

        virtual Threading::Task<bool> init() override;

        virtual std::string_view key() const override;

#ifndef MADGINE_MAINWINDOW_LAYOUT
        void renderLandingPage();
#endif
        void renderTips();
        void renderSettingsPage();
        void renderConfigurations();
        void render() override;
        void renderMenu() override;
        bool renderConfiguration(const Filesystem::Path &config) override;
        void loadConfiguration(const Filesystem::Path &config) override;
        void saveConfiguration(const Filesystem::Path &config) override;
        void renderSettings() override;

        std::vector<Tip> tips() override;

        bool mShowConfigurations = false;
        bool mShowSettings = false;

        bool mShowTipsOnStartup = true;

#ifndef MADGINE_MAINWINDOW_LAYOUT
        const Filesystem::Path &projectRoot() const;
        const std::string &projectRootString() const;
        const std::string &layout() const;

        void save();
        void load();

        void setProjectRoot(const Filesystem::Path &root);
        void setLayout(const std::string &layout);

        std::vector<std::string> projectLayouts() const;

        void setCurrentConfig(const Filesystem::Path &config);

    protected:
        void createProjectDialog();
        void openProjectDialog();

    private:
        Filesystem::Path mProjectRoot;
        std::string mLayout;
#endif

    private:
        std::set<Filesystem::Path> mConfigs;
        Filesystem::Path mCurrentConfig;

        bool mUnsavedConfiguration = false;

        Ini::IniFile mConfiguration;

        
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
