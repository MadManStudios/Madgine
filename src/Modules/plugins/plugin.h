#pragma once

#if ENABLE_PLUGINS

#    include "Interfaces/filesystem/path.h"

#    include "../threading/task.h"

#    include "Interfaces/dl/dlapi.h"

namespace Engine {
namespace Plugins {
    struct MODULES_EXPORT Plugin {
        Plugin(std::string_view name, PluginSection *section = nullptr, std::string_view project = {});
        Plugin(std::string_view name, PluginSection *section, std::string_view project, Filesystem::Path path);
        ~Plugin();

        const void *getSymbol(std::string_view name) const;

        Filesystem::Path fullPath() const;

        const std::string &project() const;

        const BinaryInfo *info() const;

        bool isDependencyOf(Plugin *p) const;

        const std::string &name() const;

        PluginSection *section() const;

        
        void ensureModule(PluginManager &manager);

        void setLoaded(bool loaded, Ini::IniFile &file);
        bool isLoaded(const Ini::IniFile &file) const;

        void loadDependencies(PluginManager &manager, Ini::IniFile &file);
        void unloadDependents(PluginManager &manager, Ini::IniFile &file);

        std::vector<std::reference_wrapper<const Plugin>> dependencies() const;
        std::vector<std::reference_wrapper<const Plugin>> dependents() const;

        friend struct PluginManager;

    protected:
        void addDependency(Plugin *dependency);
        void removeDependency(Plugin *dependency);
        void addGroupDependency(PluginSection *dependency);
        void removeGroupDependency(PluginSection *dependency);
        void clearDependencies();

        void checkCircularDependency(Plugin *dependency);
        bool checkCircularDependency(Plugin *dependency, std::vector<std::string_view> &trace);

    private:
        Dl::DlHandle mModule;

        std::string mProject;
        PluginSection *mSection;
        std::string mName;
        Filesystem::Path mPath;

        std::vector<Plugin *> mDependencies;
        std::vector<Plugin *> mDependents;
        std::vector<PluginSection *> mGroupDependencies;
    };
}
}

#endif