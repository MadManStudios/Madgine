#pragma once

namespace Engine {
namespace Ini {

    struct MODULES_EXPORT IniSection {
        IniSection();

        std::string &operator[](std::string_view key);
        std::string operator[](std::string_view key) const;

        void save(Stream &stream) const;
        void load(Stream &stream);

        std::map<std::string, std::string, std::less<>>::iterator begin();
        std::map<std::string, std::string, std::less<>>::iterator end();
        std::map<std::string, std::string, std::less<>>::const_iterator begin() const;
        std::map<std::string, std::string, std::less<>>::const_iterator end() const;

    private:
        std::map<std::string, std::string, std::less<>> mValues;
    };

}
}