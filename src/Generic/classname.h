#pragma once

#include "decay.h"
#include "stringutil.h"

#ifdef __GNUG__
#    include <cxxabi.h>
#endif

namespace Engine {
struct TypeInfo;
}

template <typename T>
extern const Engine::TypeInfo typeInfo;

namespace Engine {

template <typename T>
inline std::string typeName()
{
#ifdef __GNUG__
    int status = -4;

    std::unique_ptr<char, void (*)(void *)> res {
        abi::__cxa_demangle(typeid(T).name(), NULL, NULL, &status),
        std::free
    };

    assert(status == 0);

    return res.get();
#else
    return typeid(T).name();
#endif
}

struct TypeInfo {
    static constexpr std::string_view fix(const char *s)
    {
        //TODO <> templates
        const char *f = strrchr(s, ':');
        return f ? f + 1 : s;
    }

    template <typename T>
    constexpr TypeInfo(const char *fullName, const char *headerPath, type_holder_t<T> type)
        : mFullName(fullName)
        , mTypeName(fix(mFullName))
        , mHeaderPath(headerPath)
        , mDecayType(std::same_as<decay_t<T>, void> ? nullptr : &typeInfo<decay_t<T>>)
    {
    }

    inline std::string namespaceName() const
    {
        return std::string { mFullName, strlen(mFullName) - 2 - mTypeName.size() };
    }

    std::strong_ordering operator<=>(const TypeInfo &other) const
    {
        return strcmp(mFullName, other.mFullName);
    }

    const char *mFullName;
    std::string_view mTypeName;
    const char *mHeaderPath;
    const TypeInfo *mDecayType;
};

#define REGISTER_TYPE(T) template <> \
                        inline constexpr const ::Engine::TypeInfo typeInfo<T> = { #T, __FILE__, ::Engine::type_holder<T> };

}
