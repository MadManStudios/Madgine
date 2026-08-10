#pragma once

#include "Generic/context.h"
#include "Generic/offsetptr.h"
#include "Generic/replace.h"
#include "Generic/tag_invocable_view.h"

#include "../meta_decay.h"
#include "table_forward.h"

namespace Engine {
namespace Reflect {

    struct get_reflect_contextual_t {
        using signature = void *(Placeholder<0>, const MetaTable *);

        template <typename Context>
            requires(is_tag_invocable_v<get_reflect_contextual_t, Context, const MetaTable *>)
        void *operator()(Context &&context, const MetaTable *type) const
            noexcept(is_nothrow_tag_invocable_v<get_reflect_contextual_t, Context, const MetaTable *>)
        {
            return tag_invoke(*this, std::forward<Context>(context), type);
        }
    };

    using ContextPtr = tag_invocable_view<get_reflect_contextual_t>;

    inline constexpr get_reflect_contextual_t get_reflect_contextual;

    template <typename T, typename Context>
    struct ContextReference {

        friend void *tag_invoke(get_reflect_contextual_t, ContextReference &context, const MetaTable *type)
        {
            OffsetPtr offset { 0 };
            if (table<meta_decayed_t<T>>->isDerivedFrom(type, &offset)) {
                return reinterpret_cast<std::byte *>(&context.mValue) + offset;
            }

            return get_reflect_contextual(context.mContext, type);
        }

        Context mContext;
        T &mValue;
    };

    template <typename Context, typename T>
        requires(is_tag_invocable_v<get_reflect_contextual_t, Context, const MetaTable *> && !std::is_const_v<T>)
    ContextReference<T, Context> tag_invoke(context_set_t, Context &&context, T &value)
    {
        return { std::forward<Context>(context), value };
    }

    template <typename Context, typename T>
        requires(is_tag_invocable_v<get_reflect_contextual_t, Context, const MetaTable *>)
    T *tag_invoke(context_get_t<T>, Context &&context)
    {
        return static_cast<T *>(get_reflect_contextual(std::forward<Context>(context), table<meta_decayed_t<T>>));
    }

}
}