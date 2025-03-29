#pragma once

namespace Engine {
namespace Serialize {

    template <typename T>
    struct NoParent : T {

        template <typename... Args>
        NoParent(Args &&...args)
            : T(std::forward<Args>(args)...)
        {
            if constexpr (std::derived_from<T, TopLevelUnitBase>)
                this->sync();
            else
                setActive(*static_cast<T *>(this), true, true);
        }

        ~NoParent()
        {
            if constexpr (std::derived_from<T, TopLevelUnitBase>)
                this->unsync();
            else
                setActive(*static_cast<T *>(this), false, true);
        }

        template <typename CPO, typename... Args>
        friend auto tag_invoke(CPO f, NoParent<T> &item, Args &&...args)
            -> tag_invoke_result_t<CPO, T &, Args...>
        {
            return f(static_cast<T &>(item), std::forward<Args>(args)...);
        }
    };
}
}
