#include "../moduleslib.h"

#if ENABLE_PLUGINS

#    include "pluginmanager.h"

#    include "plugin.h"

#    include "Interfaces/dl/runtime.h"

#    include "Generic/keyvalue.h"

#    include "../ini/inisection.h"

#    include "pluginsection.h"

#    include "../ini/inifile.h"

#    include "../threading/workgroup.h"

#    include "Interfaces/filesystem/fsapi.h"

#    include "binaryinfo.h"

namespace Engine {
namespace Plugins {

    static Filesystem::Path cacheFileName()
    {
        return Filesystem::appDataPath() / ("plugins.ini");
    }

    static PluginManager *sSingleton = nullptr;

    PluginManager &PluginManager::getSingleton()
    {
        assert(sSingleton);
        return *sSingleton;
    }

    PluginManager::PluginManager()
    {
        assert(!sSingleton);
        sSingleton = this;

        PLUGIN_GROUP_DEFINITIONS;

        const std::regex e { SHARED_LIB_PREFIX "Plugin_[a-zA-Z0-9]*_([a-zA-Z0-9]*)_[a-zA-Z0-9]*\\" SHARED_LIB_SUFFIX };
        std::smatch match;
        for (auto result : Dl::listSharedLibraries()) {
            auto file = result.path().filename();
            if (std::regex_match(file.str(), match, e)) {
                std::string section = match[1];
                mSections.emplace(*this, section);
            }
        }
    }

    int PluginManager::setup(bool loadCache, std::string_view programName, const Filesystem::Path &configFile)
    {
        mUseCache = loadCache;

        assert(Threading::WorkGroup::self().singleThreaded());

        if (loadCache || !configFile.empty()) {
            Filesystem::Path pluginFile = !configFile.empty() ? Filesystem::Path { configFile } : cacheFileName();

            Ini::IniFile file;
            if (file.loadFromDisk(pluginFile)) {
                LOG("Loading Plugins from '" << pluginFile << "'");
                bool result = loadSelection(file, true);
                if (!result)
                    return -1;
            } else {
                if (!configFile.empty()) {
                    LOG_ERROR("Failed to open plugin-file '" << pluginFile << "'!");
                    return -1;
                }
            }
        }

        if (!programName.empty()) {
            Plugin exe { programName, nullptr, {}, "" };
            exe.loadDependencies(*this, mCurrentSelection);

            if (strlen(exe.info()->mToolsName) > 0) {
                section("Tools").loadPlugin(exe.info()->mToolsName, mCurrentSelection);
            }

            exe.clearDependencies();
        }

        for (PluginSection &sec : kvValues(mSections)) {
            sec.load(mCurrentSelection);
        }

        onUpdate();

        return 0;
    }

    PluginManager::~PluginManager()
    {
        assert(Threading::WorkGroup::self().singleThreaded());

        /* for (PluginSection &sec : kvValues(mSections)) {
            if (sec.unload())
                throw 0;
        }*/
    }

    PluginSection &PluginManager::section(std::string_view name)
    {
        auto pib = mSections.emplace(*this, name);
        return *pib.first;
    }

    PluginSection &PluginManager::operator[](std::string_view name)
    {
        return section(name);
    }

    const PluginSection &PluginManager::at(std::string_view name) const
    {
        return *mSections.find(name);
    }

    bool PluginManager::hasSection(std::string_view name) const
    {
        return mSections.contains(name);
    }

    Plugin *PluginManager::getPlugin(std::string_view name)
    {
        for (PluginSection &sec : kvValues(mSections)) {
            Plugin *p = sec.getPlugin(name);
            if (p)
                return p;
        }
        return nullptr;
    }

    mutable_set<PluginSection, NameCompare>::const_iterator PluginManager::begin() const
    {
        return mSections.begin();
    }

    mutable_set<PluginSection, NameCompare>::const_iterator PluginManager::end() const
    {
        return mSections.end();
    }

    mutable_set<PluginSection, NameCompare>::iterator PluginManager::begin()
    {
        return mSections.begin();
    }

    mutable_set<PluginSection, NameCompare>::iterator PluginManager::end()
    {
        return mSections.end();
    }

    void PluginManager::saveSelection(Ini::IniFile &file, bool withTools)
    {
        file = mCurrentSelection;
        if (!withTools)
            file.removeSection("Tools");
    }

    bool PluginManager::loadSelection(const Ini::IniFile &file, bool withTools)
    {
        mCurrentSelection = file;
        for (const auto &[sectionName, section] : file) {
            for (const auto &[pluginName, state] : section) {
                Plugin *plugin = getPlugin(pluginName);
                if (!plugin) {
                    LOG("Could not find Plugin \"" << pluginName << "\"!");
                    continue;
                }
                if (plugin->isLoaded(mCurrentSelection)) {
                    if (withTools) {
                        Plugin *toolsPlugin = getPlugin(pluginName + "Tools");
                        if (toolsPlugin) {
                            plugin = toolsPlugin;
                            plugin->setLoaded(true, mCurrentSelection);
                        }
                    }

                    plugin->loadDependencies(*this, mCurrentSelection);
                }
            }
        }

        for (PluginSection &sec : kvValues(mSections)) {
            sec.loadAllDependencies(mCurrentSelection);
        }

        return true;
    }

    void PluginManager::onUpdate()
    {
        if (mUseCache) {
            Ini::IniFile file;
            saveSelection(file, false);
            file.saveToDisk(cacheFileName());
        }
    }

    bool PluginManager::loadFromFile(const Filesystem::Path &p, bool withTools)
    {
        LOG("Loading Plugins: " << p);
        if (!Filesystem::exists(p))
            return false;
        Ini::IniFile file;
        if (!file.loadFromDisk(p)) {
            return false;
        }
        return loadSelection(file, withTools);
    }

    void PluginManager::saveToFile(const Filesystem::Path &p, bool withTools)
    {
        LOG("Writing Plugins: " << p);
        Ini::IniFile file;
        saveSelection(file, withTools);
        file.saveToDisk(p);
    }

    void PluginManager::setupSection(const std::string &name, bool exclusive, bool atleastOne)
    {
        auto pib = mSections.emplace(*this, name, exclusive, atleastOne);
        assert(pib.second);
    }

    const Ini::IniFile &PluginManager::selection() const
    {
        return mCurrentSelection;
    }

    Ini::IniFile &PluginManager::selection()
    {
        return mCurrentSelection;
    }

}
}

#endif