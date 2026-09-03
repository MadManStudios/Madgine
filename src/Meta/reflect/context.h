#pragma once

#include "Generic/context.h"
#include "Generic/offsetptr.h"
#include "Generic/tag_invocable_view.h"

#include "../meta_decay.h"
#include "metatable.h"
#include "table_forward.h"

namespace Engine {
namespace Reflect {

    struct get_reflect_contextual_t {
        using signature = Result(Value &, const MetaTable *);

        template <typename Context>
            requires(!is_tag_invocable_v<get_reflect_contextual_t, Context, Value &, const MetaTable *>)
        Result operator()(Context &&context, Value &retVal, const MetaTable *type) const
        {
            return REFLECT_UNKNOWN_ERROR() << "Unable to retrieve '" << type->mTypeName << "' from context!";
        }

        template <typename Context>
            requires(is_tag_invocable_v<get_reflect_contextual_t, Context, Value &, const MetaTable *>)
        Result operator()(Context &&context, Value &retVal, const MetaTable *type) const
            noexcept(is_nothrow_tag_invocable_v<get_reflect_contextual_t, Context, Value, const MetaTable *>)
        {
            return tag_invoke(*this, std::forward<Context>(context), retVal, type);
        }
    };

    inline constexpr get_reflect_contextual_t get_reflect_contextual;

    using ContextPtr = tag_invocable_view<get_reflect_contextual_t>;

    template <typename T, typename Context>
    struct ContextReference {

        friend Result tag_invoke(get_reflect_contextual_t, ContextReference &context, Value &retVal, const MetaTable *type)
        {
            if ((*toType<T>().mSecondary.mMetaTable)->isDerivedFrom(type)) {
                toValue(retVal, std::ref(context.mValue));
                return {};
            }

            return get_reflect_contextual(context.mContext, retVal, type);
        }

        Context mContext;
        T &mValue;
    };

    template <typename Context, typename T>
        requires(is_tag_invocable_v<get_reflect_contextual_t, Context, Value &, const MetaTable *> && !std::is_const_v<T>)
    ContextReference<T, Context> context_set(Context &&context, T &value)
    {
        return { std::forward<Context>(context), value };
    }

    template <typename T, typename Context>
        requires(is_tag_invocable_v<get_reflect_contextual_t, Context, Value &, const MetaTable *>)
    Result context_get(Context &&context, Value &retVal)
    {
        return get_reflect_contextual(std::forward<Context>(context), retVal, *toType<T>().mSecondary.mMetaTable);
    }

}
}