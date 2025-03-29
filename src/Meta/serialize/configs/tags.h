#pragma once

#include "configselector.h"
#include "Generic/fixed_string.h"

namespace Engine {
namespace Serialize {

    struct TagsCategory;

    template <fixed_string... TagStrings>
    struct Tags {
        using Category = TagsCategory;

        static std::array<std::string_view, sizeof...(TagStrings)> getTags() {
            return { TagStrings... };
        }
    };

    template <typename... Configs>
    using TagsSelector = ConfigSelectorDefault<TagsCategory, Tags<>, Configs...>;

}
}