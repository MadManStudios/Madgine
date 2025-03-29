#pragma once

#include "Interfaces/filesystem/path.h"
#include "inisection.h"

namespace Engine {
namespace Ini {

    struct MODULES_EXPORT IniFile {

        IniFile();
        ~IniFile();

        IniSection &operator[](std::string_view key);
        const IniSection &at(std::string_view key) const;
        bool hasSection(std::string_view key) const;
        void removeSection(std::string_view key);

        void clear();

        bool saveToDisk(const Filesystem::Path &path) const;
        bool loadFromDisk(const Filesystem::Path &path);

        std::map<std::string, IniSection, std::less<>>::iterator begin();
        std::map<std::string, IniSection, std::less<>>::iterator end();
        std::map<std::string, IniSection, std::less<>>::const_iterator begin() const;
        std::map<std::string, IniSection, std::less<>>::const_iterator end() const;

    private:
        std::map<std::string, IniSection, std::less<>> mSections;
    };

}
}