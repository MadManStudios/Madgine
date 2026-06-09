#pragma once

DLL_IMPORT_VARIABLE2(const Engine::Plugins::TypeInfo, typeInfo, typename);

namespace Engine {
namespace Plugins {

    struct TypeInfo {

        constexpr TypeInfo(std::string_view fullName)
            : mFullName(fullName)
        {
        }

        constexpr std::string_view type_name() const
        {
            size_t f = mFullName.rfind("::");
            return f == std::string_view::npos ? mFullName : mFullName.substr(f + 2);
        }

        inline std::string_view namespaceName() const
        {
            size_t f = mFullName.rfind("::");
            return f == std::string_view::npos ? "" : mFullName.substr(0, f);
        }

        std::strong_ordering operator<=>(const TypeInfo &other) const = default;

        std::string_view mFullName;
    };

#define TYPE_INFO(T) \
    std::string_view { #T }

}
}