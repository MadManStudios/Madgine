#pragma once

#include "Generic/context.h"
#include "Generic/offsetptr.h"
#include "Generic/tag_invocable_view.h"

#include "../meta_decay.h"
#include "hierarchy/serializetable_forward.h"
#include "streams/streamresult.h"

namespace Engine {
namespace Serialize {

    struct get_serialize_contextual_t {
        using signature = void *(const SerializeTable *);

        template <typename Context>
            requires(!is_tag_invocable_v<get_serialize_contextual_t, Context, const SerializeTable *>)
        void *operator()(Context &&context, const SerializeTable *type) const
        {
            return nullptr;
        }

        template <typename Context>
            requires(is_tag_invocable_v<get_serialize_contextual_t, Context, const SerializeTable *>)
        void *operator()(Context &&context, const SerializeTable *type) const
            noexcept(is_nothrow_tag_invocable_v<get_serialize_contextual_t, Context, const SerializeTable *>)
        {
            return tag_invoke(*this, std::forward<Context>(context), type);
        }
    };

    inline constexpr get_serialize_contextual_t get_serialize_contextual;

    struct get_serialize_id_t {
        using signature = ParticipantId *();

        template <typename Context>
            requires(is_tag_invocable_v<get_serialize_id_t, Context>)
        ParticipantId *operator()(Context &&context) const
            noexcept(is_nothrow_tag_invocable_v<get_serialize_contextual_t, Context>)
        {
            return tag_invoke(*this, std::forward<Context>(context));
        }
    };

    inline constexpr get_serialize_id_t get_serialize_id;

    using ContextPtr = tag_invocable_view<get_serialize_contextual_t>;

    template <typename T, typename Context>
    struct ContextReference {

        friend void *tag_invoke(get_serialize_contextual_t, ContextReference<T, Context> &context, const SerializeTable *type)
        {
            if (type == &serializeTable<meta_decayed_t<std::remove_const_t<T>>>()) {
                return const_cast<std::remove_const_t<T> *>(std::addressof(context.mValue));
            }

            return get_serialize_contextual(context.mContext, type);
        }

        friend ParticipantId *tag_invoke(get_serialize_id_t, ContextReference &context)
        {
            return get_serialize_id(context.mContext);
        }

        Context mContext;
        T &mValue;
    };

    template <typename Context, typename T>
        requires(is_tag_invocable_v<get_serialize_contextual_t, Context &, const SerializeTable *>)
    ContextReference<T, Context> context_set(Context &&context, T &value)
    {
        return { std::forward<Context>(context), value };
    }

    template <typename T, typename Context>
        requires(is_tag_invocable_v<get_serialize_contextual_t, Context, const SerializeTable *>)
    T *context_get(Context &&context)
    {
        if constexpr (std::same_as<ParticipantId, std::remove_const_t<T>>) {
            return get_serialize_id(std::forward<Context>(context));
        } else {
            return static_cast<T *>(get_serialize_contextual(std::forward<Context>(context), &serializeTable<meta_decayed_t<std::remove_const_t<std::remove_pointer_t<T>>>>()));
        }
    }

    template <typename F, typename Context>
    StreamResult context_invoke(F &&f, type_pack<>, Context &&context)
    {
        return f();
    }

    template <typename F, typename Arg, typename... Args, typename Context>
    StreamResult context_invoke(F &&f, type_pack<Contextual<Arg>, Args...>, Context &&context)
    {
        std::remove_reference_t<Arg> *arg = context_get<std::remove_reference_t<Arg>>(context);

        if (!arg)
            throw 0;

        return context_invoke([&](auto &&...args) {
            return f(*arg, std::forward<decltype(args)>(args)...);
        },
            type_pack<Args...> {}, context);
    }

    template <typename T>
    using context_contextual_filter = Concepts::is_instance<T, Contextual>;

    template <typename T>
    using context_args_filter = std::negation<context_contextual_filter<T>>;

    template <typename F>
    using context_args = CallableTraits<F>::decay_argument_types::template filter<context_args_filter>;

    template <typename F>
    using context_contextual = CallableTraits<F>::decay_argument_types::template filter<context_contextual_filter>;

    struct SyncFunctionContext {

        friend void *tag_invoke(get_serialize_contextual_t, SyncFunctionContext &context, const SerializeTable *type)
        {
            return nullptr;
        }

        friend ParticipantId *tag_invoke(get_serialize_id_t, SyncFunctionContext &context)
        {
            return &context.mId;
        }

        ParticipantId mId;
    };

}
}