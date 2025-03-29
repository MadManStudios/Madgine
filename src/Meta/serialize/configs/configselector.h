#pragma once

namespace Engine {
namespace Serialize {

    template <typename Category>
    struct CategoryChecker {
        template <typename Config>
        using check = std::is_same<typename Config::Category, Category>;
    };

    template <typename Category, typename... Configs>
    using ConfigSelector = typename type_pack<Configs...>::template filter<CategoryChecker<Category>::template check>::unpack_unique;

    template <typename Category, typename Default, typename... Configs>
    using ConfigSelectorDefault = typename type_pack<Configs...>::template filter<CategoryChecker<Category>::template check>::template unpack_unique<Default>;

    template <typename Category, typename... Configs>
    using ConfigGroupSelector = typename type_pack<Configs...>::template filter<CategoryChecker<Category>::template check>;
}
}