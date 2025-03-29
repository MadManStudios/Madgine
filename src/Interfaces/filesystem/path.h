#pragma once

namespace Engine {
namespace Filesystem {

    struct INTERFACES_EXPORT Path {

        Path() = default;
        Path(std::string_view s);  
        Path(const std::string &s);
        Path(const char *s);

		Path &operator=(std::string_view s);        
        Path &operator=(const std::string &s);        
        Path &operator=(const char *s);        

        Path &operator/=(const Path &p);

        Path operator/(const Path &p) const
        {
            Path p2 = *this;
            p2 /= p;
            return p2;
        }

        bool operator==(const Path &other) const;
        bool operator!=(const Path &other) const;
        std::strong_ordering operator<=>(const Path &other) const noexcept = default;

        Path parentPath() const;
        Path relative() const;
        Path relative(const Path &base) const;
        Path absolute() const;
        Path absolute(const Path &base) const;

		void normalize();

        Path filename() const;
        std::string_view stem() const;
        std::string_view extension() const;

        bool isAbsolute() const;
        bool isRelative() const;
        bool isRelative(const Path &base) const;

		void clear();
        bool empty() const;
        const std::string &str() const;
        const char *c_str() const;

        operator const std::string &() const;
        operator std::string_view() const;

    private:
        std::string mPath;
    };

    inline std::ostream &operator<<(std::ostream &out, const Path &p)
    {
        return out << p.str();
    }

}
}