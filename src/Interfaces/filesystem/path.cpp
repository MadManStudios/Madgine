#include "../interfaceslib.h"
#include "path.h"
#include "fsapi.h"

namespace Engine {
namespace Filesystem {

    Path::Path(std::string_view s)
        : mPath(s)
    {
        normalize();
    }

    Path::Path(const std::string &s)
        : Path(std::string_view { s })
    {
    }

    Path::Path(const char *s)
        : Path(std::string_view { s })
    {
    }

    Path &Path::operator=(std::string_view s)
    {
        mPath = s;
        normalize();
        return *this;
    }

    Path &Path::operator=(const std::string &s)
    {
        return operator=(std::string_view { s });
    }

    Path &Path::operator=(const char *s)
    {
        return operator=(std::string_view { s });
    }

    Path &Path::operator/=(const Path &other)
    {
        assert(other.isRelative());
        if (!mPath.empty()) {
            mPath.reserve(mPath.size() + other.mPath.size() + 1);
            mPath.push_back('/');
        }
        mPath.append(other.mPath);
        normalize();
        return *this;
    }

    bool Path::operator==(const Path &other) const
    {
        return mPath == other.mPath;
    }

    bool Path::operator!=(const Path &other) const
    {
        return mPath != other.mPath;
    }

    Path Path::parentPath() const
    {
        auto sep = mPath.rfind('/');
        if (sep == std::string::npos)
            return {};
        std::string result = mPath.substr(0, sep);
        if (result.empty())
            result = "/";
        return result;
    }

    Path Path::relative() const
    {
        return relative(getCwd());
    }

    Path Path::relative(const Path &base) const
    {
        size_t baseCount = base.mPath.size();
        size_t count = mPath.size();

        if (baseCount >= count)
            return {};

        if (!isEqual(mPath.substr(0, baseCount), base.mPath))
            return {};

        if (baseCount == count)
            return ".";

        if (mPath[baseCount] != '/')
            return {};

        return mPath.substr(baseCount + 1);
    }

    Path Path::absolute() const
    {
        return absolute(getCwd());
    }

    Path Path::absolute(const Path &base) const
    {
        if (isAbsolute())
            return *this;
        else
            return base / *this;
    }

    void Path::normalize()
    {
        if (!isValidPath(mPath)) {
            mPath.clear();
            return;
        }

        std::string lastElement;

        size_t size = mPath.size();
        size_t cursor = 0;

        bool hadSeparator = false;
        bool canGoUp = false;
        for (size_t i = 0; i < size; ++i) {
            if (isSeparator(mPath[i])) {
                if (!hadSeparator) {
                    hadSeparator = lastElement.empty() || lastElement.back() != ':';
                    if (lastElement == ".." && canGoUp) {
                        --cursor;
                        cursor = mPath.rfind('/', cursor - 1) + 1;
                    } else if (lastElement != ".") {
                        mPath.replace(cursor, lastElement.size(), lastElement.c_str());
                        cursor += lastElement.size();
                        mPath.replace(cursor, 1, "/");
                        ++cursor;
                        canGoUp = (lastElement != "..");
                    }
                    lastElement.clear();
                }
            } else {
                hadSeparator = false;
                lastElement.push_back(mPath[i]);
            }
        }

        if (!lastElement.empty()) {            
            if (lastElement == ".." && cursor > 0) {
                --cursor;
                size_t pos = mPath.rfind('/', cursor - 1);
                if (pos != std::string::npos) {
                    cursor = std::max(pos, size_t { 1 });
                } else {
                    cursor = 0;
                }
            } else if (lastElement != "." || cursor == 0) {
                mPath.replace(cursor, lastElement.size(), lastElement.c_str());
                cursor += lastElement.size();
            }
        }

        if (hadSeparator && cursor > 1) {
            --cursor;
        }

        mPath.resize(cursor);

        //Filesystem::makeNormalized(mPath);
    }

    Path Path::filename() const
    {
        auto sep = mPath.rfind('/');
        return sep == std::string::npos ? mPath : mPath.substr(sep + 1);
    }

    std::string_view Path::stem() const
    {
        auto sep = mPath.rfind('/');
        auto pos = mPath.rfind('.');
        if (pos != std::string::npos) {
            if (sep != std::string::npos) {
                if (pos > sep)
                    return { mPath.data() + sep + 1, pos - sep - 1 };
                else
                    return { mPath.data() + sep + 1 };
            } else {
                return { mPath.data(), pos };
            }
        } else {
            if (sep != std::string::npos) {
                return { mPath.data() + sep + 1 };
            } else {
                return mPath;
            }
        }
    }

    std::string_view Path::extension() const
    {
        auto sep = mPath.rfind('/');
        auto pos = mPath.rfind('.');
        if (pos != std::string::npos) {
            if (sep != std::string::npos) {
                if (pos > sep)
                    return { mPath.data() + pos };
                else
                    return {};
            } else {
                return { mPath.data() + pos };
            }
        } else {
            return {};
        }
    }

    std::string_view Path::protocol() const
    {
        size_t separator = mPath.find("://");
        std::string_view protocol;
        if (separator != std::string::npos) {
            protocol = std::string_view { mPath.c_str(), separator };
        }
        return protocol;
    }

    bool Path::isAbsolute() const
    {
        return Filesystem::isAbsolute(*this);
    }

    bool Path::isRelative() const
    {
        return !isAbsolute();
    }

    bool Path::isRelative(const Path &base) const
    {
        auto [firstEnd, secondEnd] = std::mismatch(mPath.begin(), mPath.end(), base.str().begin(), base.str().end());
        return secondEnd == base.str().end();
    }

    void Path::clear()
    {
        mPath.clear();
    }

    bool Path::empty() const
    {
        return mPath.empty();
    }

    const std::string &Path::str() const
    {
        return mPath;
    }

    const char *Path::c_str() const
    {
        return mPath.c_str();
    }

    Path::operator const std::string &() const
    {
        return mPath;
    }

    Path::operator std::string_view() const
    {
        return mPath;
    }

}
}