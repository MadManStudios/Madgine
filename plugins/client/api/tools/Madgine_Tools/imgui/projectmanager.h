#pragma once

#include "Generic/execution/signal.h"

#include "Platform/filesystem/path.h"

#include "Modules/plugins/inifile.h"

#include "Madgine/window/layoutloader.h"

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"
#include "Madgine_Tools/util/undostack.h"

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

        void loadConfiguration(const Platform::Filesystem::Path &config) override;
        void saveConfiguration(const Platform::Filesystem::Path &config) override;

#endif
        void renderTips();
        void renderSettingsPage();
        void render() override;
        void renderMenu() override;
        bool renderConfiguration(const Platform::Filesystem::Path &config) override;
        void renderSettings() override;

        void renderLayoutDetails();

        std::vector<Tip> tips() override;

        bool mShowSettings = false;

        bool mShowTipsOnStartup = true;

#ifndef MADGINE_MAINWINDOW_LAYOUT
        void save();
        void load();

        Core::LayoutLoader::Resource *layout() const;
        std::string_view layoutString() const;
        void setLayoutString(std::string_view name);
        void setLayout(Core::LayoutLoader::Resource *layout);

        void setCurrentConfig(const Platform::Filesystem::Path &config);

    protected:
        void createProjectDialog();
        void createPluginDialog();

    private:
        Core::LayoutLoader::Resource *mLayout = nullptr;

        bool mShowConfigurations = false;

        std::set<Platform::Filesystem::Path> mConfigs;
        Platform::Filesystem::Path mCurrentConfig;

        bool mUnsavedConfiguration = false;

        Plugins::IniFile mConfiguration;
#endif

    private:
        bool mShowTips = false;
        bool mInitialized = false;

        std::vector<Tip> mTips;
        size_t mTipIndex = 0;

    private:
        Core::MainWindow *mWindow = nullptr;
        Templates *mTemplates = nullptr;
        Inspector *mInspector = nullptr;

        bool mLayoutDetailsVisible = true;

        UndoStack mHistory;
    };

}
}
