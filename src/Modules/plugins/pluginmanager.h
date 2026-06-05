#pragma once

#if ENABLE_PLUGINS

#    include "Generic/containers/mutable_set.h"

#    include "inifile.h"
#    include "namecompare.h"

namespace Engine {
namespace Plugins {

    struct MODULES_EXPORT PluginManager {
        static PluginManager &getSingleton();

        PluginManager();
        PluginManager(const PluginManager &) = delete;
        ~PluginManager();

        PluginManager &operator=(const PluginManager &) = delete;

        int setup(bool loadCache, std::string_view programName, const Platform::Filesystem::Path &configFile);

        PluginSection &section(std::string_view name);
        PluginSection &operator[](std::string_view name);
        const PluginSection &at(std::string_view name) const;
        bool hasSection(std::string_view name) const;

        Plugin *getPlugin(std::string_view name);

        Containers::mutable_set<PluginSection, NameCompare>::const_iterator begin() const;
        Containers::mutable_set<PluginSection, NameCompare>::const_iterator end() const;
        Containers::mutable_set<PluginSection, NameCompare>::iterator begin();
        Containers::mutable_set<PluginSection, NameCompare>::iterator end();

        bool loadFromFile(const Platform::Filesystem::Path &p, bool withTools);
        void saveToFile(const Platform::Filesystem::Path &p, bool withTools);

        void saveSelection(IniFile &file, bool withTools);
        bool loadSelection(const IniFile &file, bool withTools);

        const IniFile &selection() const;
        IniFile &selection();

    protected:
        void setupSection(const std::string &name, bool exclusive, bool atleastOne);

        void onUpdate();

        friend struct PluginSection;

    private:
        Containers::mutable_set<PluginSection, NameCompare> mSections;

        IniFile mCurrentSelection;

        bool mUseCache = false;
    };

}

}

#endif
