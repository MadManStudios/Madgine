#pragma once

#if ENABLE_PLUGINS

#    include "Generic/containers/mutable_set.h"

#    include "../threading/taskfuture.h"
#    include "namecompare.h"

namespace Engine {
namespace Plugins {

    struct MODULES_EXPORT PluginSection {
        PluginSection(PluginManager &mgr, std::string_view name, bool exclusive = false, bool atleastOne = false);
        PluginSection(const PluginSection &) = delete;
        ~PluginSection();

        const std::string &name() const;

        bool isAtleastOne() const;
        bool isExclusive() const;

        bool load(IniFile &file);
        bool unload(IniFile &file);

        bool isLoaded(std::string_view name, IniFile &file);
        bool loadPlugin(std::string_view name, IniFile &file);
        bool unloadPlugin(std::string_view name, IniFile &file);

        bool loadPluginByFilename(std::string_view name, IniFile &file);

        void loadAllDependencies(IniFile &file);

        template <typename T>
        T *getUniqueSymbol(std::string_view name) const
        {
            return static_cast<T *>(getUniqueSymbol(name));
        }
        const void *getUniqueSymbol(std::string_view name) const;

        Containers::mutable_set<Plugin, NameCompare>::const_iterator begin() const;
        Containers::mutable_set<Plugin, NameCompare>::const_iterator end() const;
        Containers::mutable_set<Plugin, NameCompare>::iterator begin();
        Containers::mutable_set<Plugin, NameCompare>::iterator end();

        PluginManager &manager();

        Plugin *getPlugin(std::string_view name);

    private:
        bool loadPlugin(Plugin *p, IniFile &file, bool autoLoadTools = true);
        bool unloadPlugin(Plugin *p, IniFile &file);

        friend struct Plugin;

        Containers::mutable_set<Plugin, NameCompare> mPlugins;

        std::vector<Plugin *> mDependents;

        std::string mName;

        PluginManager &mMgr;

        const bool mAtleastOne;
        const bool mExclusive;
    };

}
}

#endif