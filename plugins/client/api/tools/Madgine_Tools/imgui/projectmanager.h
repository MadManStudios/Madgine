#pragma once

#include "Madgine_Tools/toolscollector.h"
#include "Madgine_Tools/toolbase.h"

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

        virtual void render() override;
        virtual void renderMenu() override;
        virtual bool renderConfiguration(const Filesystem::Path &config) override;
        virtual void loadConfiguration(const Filesystem::Path &config) override;
        virtual void saveConfiguration(const Filesystem::Path &config) override;

        const Filesystem::Path &projectRoot() const;
        const std::string &projectRootString() const;
        const std::string &layout() const;

        void save();
        void load();

        void setProjectRoot(const Filesystem::Path &root);
        void setLayout(const std::string &layout);

        std::vector<std::string> projectLayouts() const;

        void setCurrentConfig(const Filesystem::Path &config);
        
        bool mShowConfigurations = false;
        bool mShowSettings = false;

    private:
        std::set<Filesystem::Path> mConfigs;
        Filesystem::Path mCurrentConfig;        

        bool mUnsavedConfiguration = false;

        Ini::IniFile mConfiguration;

    private:
        Window::MainWindow *mWindow = nullptr;
        Filesystem::Path mProjectRoot;
        std::string mLayout;
    };

}
}

REGISTER_TYPE(Engine::Tools::ProjectManager)