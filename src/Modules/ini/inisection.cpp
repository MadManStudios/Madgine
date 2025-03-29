#include "../moduleslib.h"

#include "inisection.h"

#include "Generic/stream.h"

namespace Engine {
namespace Ini {

    IniSection::IniSection()
    {
    }

    std::string &IniSection::operator[](std::string_view key)
    {
        return mValues[std::string{ key }];
    }

    std::string IniSection::operator[](std::string_view key) const
    {
        if (!mValues.contains(std::string { key }))
            return "";
        return mValues.at(std::string { key });
    }

    void IniSection::save(Stream &stream) const
    {
        for (const std::pair<const std::string, std::string> &p : mValues) {
            stream << p.first << "=" << p.second << '\n';
        }
    }

    void IniSection::load(Stream &stream)
    {
        mValues.clear();
        std::streampos save = stream.tell();
        std::string line;
        while (std::getline(stream.stream(), line)) {
            if (line.empty()) {
                continue;
            }
            if (line.at(0) == '[') {
                stream.seek(save);
                return;
            }
            size_t pos = line.find('=');
            if (pos == std::string::npos || line.rfind('=') != pos)
                std::terminate();
            mValues[line.substr(0, pos)] = line.substr(pos + 1);
            save = stream.tell();
        }
    }

    std::map<std::string, std::string, std::less<>>::iterator IniSection::begin()
    {
        return mValues.begin();
    }

    std::map<std::string, std::string, std::less<>>::iterator IniSection::end()
    {
        return mValues.end();
    }

    std::map<std::string, std::string, std::less<>>::const_iterator IniSection::begin() const
    {
        return mValues.begin();
    }

    std::map<std::string, std::string, std::less<>>::const_iterator IniSection::end() const
    {
        return mValues.end();
    }

}
}