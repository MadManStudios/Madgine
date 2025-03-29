#include "../moduleslib.h"

#if ENABLE_PLUGINS

#    include "Generic/projections.h"

#    include "plugin.h"

#    include "binaryinfo.h"

#    include "pluginmanager.h"
#    include "pluginsection.h"

namespace Engine {
namespace Plugins {

    Plugin::Plugin(std::string_view name, PluginSection *section, std::string_view project)
        : mProject(project)
        , mSection(section)
        , mName(name)
#    if WINDOWS
        , mPath(mName)
#    elif UNIX
        , mPath("lib" + mName + ".so")
#    endif
    {
        assert(!mName.empty());
    }

    Plugin::Plugin(std::string_view name, PluginSection *section, std::string_view project, Filesystem::Path path)
        : mProject(project)
        , mSection(section)
        , mName(name)
        , mPath(std::move(path))
    {
        assert(!mName.empty());
    }

    Plugin::~Plugin()
    {
        //assert(!mModule);
        //assert(mDependencies.empty());
        //assert(mDependents.empty());
    }

    void Plugin::ensureModule(PluginManager &manager)
    {
        if (!mModule) {
            std::string errorMsg;
            Dl::DlAPIResult result = mModule.open(mPath);
            if (result == Dl::DlAPIResult::SUCCESS) {
                const BinaryInfo *bin = info();
                if (bin) {
                    bin->mSelf = this;

                    for (const char **dep = bin->mPluginDependencies; *dep; ++dep) {
                        addDependency(manager.getPlugin(*dep));
                    }

                    for (const char **dep = bin->mPluginGroupDependencies; *dep; ++dep) {
                        addGroupDependency(&manager.section(*dep));
                    }
                } else {
                    LOG_ERROR("Unable to locate BinaryInfo. Make sure you call generate_binary_info in your CMakeLists.txt for your binaries!");
                    std::terminate();
                }
            } else {
                LOG_ERROR(result.toString() << " loading dynamic library '" << mPath
                                            << "'");
                std::terminate();
            }
        }
    }

    void Plugin::loadDependencies(PluginManager &manager, Ini::IniFile &file)
    {
        ensureModule(manager);
        for (Plugin *dep : mDependencies) {
            if (!dep->isLoaded(file)) {
                if (mName == dep->mName + "Tools") {
                    dep->mSection->loadPlugin(dep, file, false);
                } else {
                    dep->mSection->loadPlugin(dep, file);
                }
            }
        }
        for (PluginSection *sec : mGroupDependencies) {
            sec->load(file);
        }
    }

    void Plugin::unloadDependents(PluginManager &manager, Ini::IniFile &file)
    {
        assert(isLoaded(file));
        ensureModule(manager);
        for (Plugin *dep : mDependents) {
            if (dep->mSection->unloadPlugin(dep, file)) {
                LOG_ERROR("Unload of dependent for plugin \"" << mName << "\" failed!");
                throw 0;
            }
        }
    }

    const void *Plugin::getSymbol(std::string_view name) const
    {
        std::string fullName = std::string { name } + "_" + mName;
        return mModule.getSymbol(fullName);
    }

    Filesystem::Path Plugin::fullPath() const
    {

        std::string fullName = "binaryInfo_" + mName;
        return mModule.fullPath(fullName);
    }

    const std::string &Plugin::project() const
    {
        return mProject;
    }

    const BinaryInfo *Plugin::info() const
    {
        assert(mModule);
        return static_cast<const BinaryInfo *>(getSymbol("binaryInfo"));
    }

    bool Plugin::isDependencyOf(Plugin *p) const
    {
        if (p == this)
            return true;
        for (Plugin *p2 : mDependents) {
            if (p2->isDependencyOf(p))
                return true;
        }
        return false;
    }

    const std::string &Plugin::name() const
    {
        return mName;
    }

    PluginSection *Plugin::section() const
    {
        return mSection;
    }

    bool Plugin::isLoaded(const Ini::IniFile &file) const
    {
        if (!file.hasSection(mSection->name()))
            return false;
        return !file.at(mSection->name())[mName].empty();
    }

    void Plugin::setLoaded(bool loaded, Ini::IniFile &file)
    {
        std::string &value = file[mSection->name()][mName];
        if (!value.empty() != loaded) {
            LOG("Plugin " << mName << "... " << (loaded ? "Loaded" : "Unloaded"));
            value = loaded ? "On" : "";
        }
    }

    void Plugin::addDependency(Plugin *dependency)
    {
        checkCircularDependency(dependency);
        mDependencies.push_back(dependency);
        dependency->mDependents.push_back(this);
    }

    void Plugin::removeDependency(Plugin *dependency)
    {
        auto it = std::ranges::find(mDependencies, dependency);
        assert(it != mDependencies.end());

        auto it2 = std::ranges::find(dependency->mDependents, this);
        assert(it2 != dependency->mDependents.end());

        mDependencies.erase(it);
        dependency->mDependents.erase(it2);
    }

    void Plugin::addGroupDependency(PluginSection *dependency)
    {
        for (Plugin &p : *dependency) {
            checkCircularDependency(&p);
        }
        mGroupDependencies.push_back(dependency);
        dependency->mDependents.push_back(this);
    }

    void Plugin::removeGroupDependency(PluginSection *dependency)
    {
        auto it = std::ranges::find(mGroupDependencies, dependency);
        assert(it != mGroupDependencies.end());

        auto it2 = std::ranges::find(dependency->mDependents, this);
        assert(it2 != dependency->mDependents.end());

        mGroupDependencies.erase(it);
        dependency->mDependents.erase(it2);
    }

    void Plugin::clearDependencies()
    {
        while (!mDependencies.empty()) {
            Plugin *dependency = mDependencies.back();

            auto it = std::ranges::find(dependency->mDependents, this);
            assert(it != dependency->mDependents.end());

            mDependencies.pop_back();
            dependency->mDependents.erase(it);
        }

        while (!mGroupDependencies.empty()) {
            PluginSection *dependency = mGroupDependencies.back();

            auto it = std::ranges::find(dependency->mDependents, this);
            assert(it != dependency->mDependents.end());

            mGroupDependencies.pop_back();
            dependency->mDependents.erase(it);
        }

        assert(mModule);
        mModule.close();
    }

    void Plugin::checkCircularDependency(Plugin *dependency)
    {
        std::vector<std::string_view> trace { mName };
        if (checkCircularDependency(dependency, trace)) {
            LOG_ERROR("Detected Circular Plugin-Dependency: " << StringUtil::join(trace, " -> "));
            throw 0;
        }
    }

    bool Plugin::checkCircularDependency(Plugin *dependency, std::vector<std::string_view> &trace)
    {
        trace.push_back(dependency->mName);
        if (dependency == this)
            return true;

        for (Plugin *p : dependency->mDependencies) {
            if (checkCircularDependency(p, trace))
                return true;
        }

        for (PluginSection *sec : dependency->mGroupDependencies) {
            trace.push_back(sec->mName);
            for (Plugin &p : *sec) {
                if (checkCircularDependency(&p, trace))
                    return true;
            }
            trace.pop_back();
        }
        trace.pop_back();

        return false;
    }
}
}

#endif