#include "../moduleslib.h"

#if ENABLE_PLUGINS

#    include "pluginsection.h"

#    include "plugin.h"

#    include "pluginmanager.h"

#    include "Interfaces/dl/runtime.h"

#    include "Generic/keyvalue.h"

#    include "../ini/inisection.h"

#    include "../threading/workgroup.h"

#    include "binaryinfo.h"

namespace Engine {
namespace Plugins {

    PluginSection::PluginSection(PluginManager &mgr, std::string_view name, bool exclusive, bool atleastOne)
        : mName(name)
        , mMgr(mgr)
        , mAtleastOne(atleastOne)
        , mExclusive(exclusive)
    {
        const std::regex e { SHARED_LIB_PREFIX "Plugin_([a-zA-Z0-9]*)_" + mName + "_([a-zA-Z0-9]*)\\" SHARED_LIB_SUFFIX };
        std::smatch match;
        for (auto result : Dl::listSharedLibraries()) {
            auto file = result.path().filename();
            if (std::regex_match(file.str(), match, e)) {
                std::string project = match[1];
                std::string name = match[2];
                auto pib = mPlugins.emplace(name, this, project, result.path());
                assert(pib.second);
            }
        }
    }

    PluginSection::~PluginSection()
    {
        //assert(mDependents.empty());
    }

    const std::string &PluginSection::name() const
    {
        return mName;
    }

    bool PluginSection::isAtleastOne() const
    {
        return mAtleastOne;
    }

    bool PluginSection::isExclusive() const
    {
        return mExclusive;
    }

    bool PluginSection::load(Ini::IniFile &file)
    {
        if (mAtleastOne) {
            if (mPlugins.empty()) {
                LOG_ERROR("No plugin available in Section tagged as atleastOne: " << mName);
                std::terminate();
            }
            for (Plugin &p : kvValues(mPlugins)) {
                if (p.isLoaded(file))
                    return true;
            }
            return loadPlugin(&*mPlugins.begin(), file);
        }
        return true;
    }

    bool PluginSection::unload(Ini::IniFile &file)
    {
        for (Plugin *p : mDependents) {
            if (!p->section()->unloadPlugin(p, file))
                return true;
        }

        for (Plugin &p : kvValues(mPlugins)) {
            if (unloadPlugin(&p, file))
                return true;
        }

        return false;
    }

    bool PluginSection::isLoaded(std::string_view name, Ini::IniFile &file)
    {
        auto it = mPlugins.find(name);
        if (it != mPlugins.end()) {
            return it->isLoaded(file);
        }
        return false;
    }

    bool PluginSection::loadPlugin(std::string_view name, Ini::IniFile &file)
    {
        Plugin *plugin = getPlugin(name);
        if (!plugin)
            return false;
        return loadPlugin(plugin, file);
    }

    bool PluginSection::unloadPlugin(std::string_view name, Ini::IniFile &file)
    {
        Plugin *plugin = getPlugin(name);
        if (!plugin)
            return false;
        return unloadPlugin(plugin, file);
    }

    bool PluginSection::loadPluginByFilename(std::string_view name, Ini::IniFile &file)
    {
        auto pib = mPlugins.emplace(name);
        assert(pib.second);
        return loadPlugin(&*pib.first, file);
    }

    Plugin *PluginSection::getPlugin(std::string_view name)
    {
        auto it = mPlugins.find(name);
        if (it == mPlugins.end())
            return nullptr;
        return &*it;
    }

    bool PluginSection::loadPlugin(Plugin *p, Ini::IniFile &file, bool autoLoadTools)
    {
        assert(p->section() == this);

        if (mExclusive) {
            Plugin *unloadExclusive = nullptr;

            for (Plugin &p2 : kvValues(mPlugins)) {
                if (&p2 != p && p2.isLoaded(file)) {
                    assert(!unloadExclusive);
                    unloadExclusive = &p2;
                }
            }

            if (unloadExclusive) {
                unloadExclusive->unloadDependents(mMgr, file);
                unloadExclusive->setLoaded(false, file);
            }
        }

        p->setLoaded(true, file);
        p->loadDependencies(mMgr, file);

        
        if (autoLoadTools && strlen(p->info()->mToolsName) > 0) {
            PluginSection &toolsSection = mMgr.section("Tools");
            return toolsSection.loadPlugin(p->info()->mToolsName, file);
        } else {
            mMgr.onUpdate();
            return true;
        }
    }

    bool PluginSection::unloadPlugin(Plugin *p, Ini::IniFile &file)
    {
        assert(!mAtleastOne);

        p->unloadDependents(mMgr, file);
        p->setLoaded(false, file);

        return false;
    }

    mutable_set<Plugin, NameCompare>::const_iterator PluginSection::begin() const
    {
        return mPlugins.begin();
    }

    mutable_set<Plugin, NameCompare>::const_iterator PluginSection::end() const
    {
        return mPlugins.end();
    }

    mutable_set<Plugin, NameCompare>::iterator PluginSection::begin()
    {
        return mPlugins.begin();
    }

    mutable_set<Plugin, NameCompare>::iterator PluginSection::end()
    {
        return mPlugins.end();
    }

    void PluginSection::loadAllDependencies(Ini::IniFile &file)
    {
        for (Plugin &p : kvValues(mPlugins)) {
            if (p.isLoaded(file))
                p.loadDependencies(mMgr, file);
        }
    }

    PluginManager &PluginSection::manager()
    {
        return mMgr;
    }

    const void *PluginSection::getUniqueSymbol(std::string_view name) const
    {
        const void *symbol = nullptr;
        for (const Plugin &p : mPlugins) {
            const void *s = p.getSymbol(name);
            if (s) {
                if (symbol)
                    std::terminate();
                symbol = s;
            }
        }
        return symbol;
    }

}
}

#endif