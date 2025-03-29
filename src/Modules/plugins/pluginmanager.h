#pragma once

#if ENABLE_PLUGINS

#    include "../ini/inifile.h"

#    include "Generic/container/mutable_set.h"

#    include "namecompare.h"

namespace Engine {
namespace Plugins {

    struct MODULES_EXPORT PluginManager {
        static PluginManager &getSingleton();

        PluginManager();
        PluginManager(const PluginManager &) = delete;
        ~PluginManager();

        PluginManager &operator=(const PluginManager &) = delete;

        int setup(bool loadCache, std::string_view programName, const Filesystem::Path &configFile);

        PluginSection &section(std::string_view name);
        PluginSection &operator[](std::string_view name);
        const PluginSection &at(std::string_view name) const;
        bool hasSection(std::string_view name) const;

        Plugin *getPlugin(std::string_view name);

        mutable_set<PluginSection, NameCompare>::const_iterator begin() const;
        mutable_set<PluginSection, NameCompare>::const_iterator end() const;
        mutable_set<PluginSection, NameCompare>::iterator begin();
        mutable_set<PluginSection, NameCompare>::iterator end();

        bool loadFromFile(const Filesystem::Path &p, bool withTools);
        void saveToFile(const Filesystem::Path &p, bool withTools);

        void saveSelection(Ini::IniFile &file, bool withTools);
        bool loadSelection(const Ini::IniFile &file, bool withTools);

        const Ini::IniFile &selection() const;
        Ini::IniFile &selection();

    protected:
        void setupSection(const std::string &name, bool exclusive, bool atleastOne);

        void onUpdate();

        friend struct PluginSection;

    private:
        mutable_set<PluginSection, NameCompare> mSections;

        Ini::IniFile mCurrentSelection;

        bool mUseCache = false;
    };

}

}

#endif
