#pragma once

#if ENABLE_PLUGINS

#    include "Platform/dl/dlapi.h"
#    include "Platform/filesystem/path.h"

namespace Engine {
namespace Plugins {
    struct MODULES_EXPORT Plugin {
        Plugin(std::string_view name, PluginSection *section = nullptr, std::string_view project = {});
        Plugin(std::string_view name, PluginSection *section, std::string_view project, Platform::Filesystem::Path path);
        ~Plugin();

        const void *getSymbol(std::string_view name) const;

        Platform::Filesystem::Path fullPath() const;

        const std::string &project() const;

        const BinaryInfo *info() const;

        bool isDependencyOf(Plugin *p) const;

        const std::string &name() const;

        PluginSection *section() const;

        void ensureModule(PluginManager &manager);

        void setLoaded(bool loaded, IniFile &file);
        bool isLoaded(const IniFile &file) const;

        void loadDependencies(PluginManager &manager, IniFile &file);
        void unloadDependents(PluginManager &manager, IniFile &file);

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
        Platform::Dl::DlHandle mModule;

        std::string mProject;
        PluginSection *mSection;
        std::string mName;
        Platform::Filesystem::Path mPath;

        std::vector<Plugin *> mDependencies;
        std::vector<Plugin *> mDependents;
        std::vector<PluginSection *> mGroupDependencies;
    };
}
}

#endif