#pragma once

#include "Generic/callable_view.h"
#include "Generic/execution/concepts.h"
#include "Generic/execution/storage.h"
#include "Generic/fixed_string.h"

#include "Meta/reflect/util.h"
#include "Meta/serialize/operations.h"
#include "Meta/serialize/streams/streamresult.h"

namespace Engine {
namespace Behavior {

    template <typename Rec, typename T>
    struct ContextParameterState : Execution::base_state<Rec> {

        ContextParameterState(ContextParameter<T> value, Rec &&rec)
            : Execution::base_state<Rec>(std::forward<Rec>(rec))
            , mValue(std::move(value))

        {
        }

        void start()
        {
            if (!mValue.mValue) {
                Reflect::Result result = mValue.resolve(this->mRec);
                if (result) {
                    this->mRec.set_error(std::move(*result.mError));
                    return;
                }
            }

            this->mRec.set_value(*mValue.mValue);
        }

        void stop()
        {
        }

        friend auto tag_invoke(Execution::visit_state_t, ContextParameterState *state, auto &&visitor)
        {
        }

        ContextParameter<T> mValue;
    };

    template <typename T>
    struct ContextParameter {

        ContextParameter() = default;

        ContextParameter(T &&value)
            : mValue(std::in_place, std::forward<T>(value))
        {
        }

        ContextParameter(std::optional<forward_ref_t<T>> value)
            : mValue(std::move(value))
        {
        }

        using is_sender = void;

        using result_type = Reflect::Error;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<T &>;

        Reflect::Result resolve(auto &context)
        {
            Reflect::Result result;
            if (!mValue) {
                Reflect::Value_erased([&](Reflect::Value &v) {
                    result = Reflect::get_reflect_contextual(context, v, *Reflect::toType<std::decay_t<T>>().mSecondary.mMetaTable);
                    if (!result) {
                        result = Reflect::call([&](T val) -> Reflect::Result {
                            mValue.emplace(std::forward<T>(val));
                            return {};
                        },
                            v);
                    }
                });
            }
            return result;
        }

        T &operator->()
        {
            return *mValue;
        }

        T &operator*()
        {
            return *mValue;
        }

        decltype(auto) operator->*(auto &&arg)
        {
            return *mValue->*std::forward<decltype(arg)>(arg);
        }

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, ContextParameter &&sender, Rec &&rec)
        {
            return ContextParameterState<Rec, T> { std::move(sender), std::forward<Rec>(rec) };
        }

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, ContextParameter &sender, Rec &&rec)
        {
            return ContextParameterState<Rec, T> { sender, std::forward<Rec>(rec) };
        }

        using meta_t = std::optional<forward_ref_t<T>>;

        template <bool isReferenceWrapped>
        friend decltype(auto) tag_invoke(Reflect::convert_Value_t<isReferenceWrapped> convert_ValueType, ContextParameter<T> &named)
        {
            return convert_ValueType(named.mValue);
        }

        template <bool isReferenceWrapped>
        friend decltype(auto) tag_invoke(Reflect::convert_Value_t<isReferenceWrapped> convert_ValueType, ContextParameter<T> &&named)
        {
            return convert_ValueType(std::move(named.mValue));
        }

        template <typename Context>
        friend Serialize::StreamResult tag_invoke(Serialize::apply_map_t, ContextParameter<T> &named, Serialize::FormattedSerializeStream &in, bool success, Context &&context)
        {
            if (named.mValue)
                return Serialize::apply_map(*named.mValue, in, success, context);
            else
                return {};
        }

        std::optional<forward_ref_t<T>> mValue;
    };

    struct context_set_t {

        template <typename Rec, typename T>
        struct receiver : Execution::algorithm_receiver<Rec> {

            receiver(Rec &&rec, T &&value)
                : Execution::algorithm_receiver<Rec> { std::forward<Rec>(rec) }
                , mValue(std::forward<T>(value))
            {
            }

            friend Reflect::Result tag_invoke(Reflect::get_reflect_contextual_t, receiver &rec, Reflect::Value &retVal, const Reflect::MetaTable *type)
            {
                if ((*Reflect::toType<std::decay_t<T>>().mSecondary.mMetaTable)->isDerivedFrom(type)) {
                    toValue(retVal, forward_ref<T>(rec.mValue));
                    return {};
                }

                return Reflect::get_reflect_contextual(rec.mRec, retVal, type);
            }

            T mValue;
        };

        template <Execution::AnySender Inner, typename T>
        struct sender : Execution::algorithm_sender<Inner> {
            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, sender &&sender, Rec &&rec)
            {
                return Execution::algorithm_state<Inner, receiver<Rec, T>> { std::forward<Inner>(sender.mSender), receiver<Rec, T> { std::forward<Rec>(rec), std::forward<T>(sender.mValue) } };
            }

            T mValue;
        };

        template <Execution::AnySender Sender, typename T>
        friend auto tag_invoke(context_set_t, Sender &&inner, T &&value)
        {
            return sender<Sender, T> { { {}, std::forward<Sender>(inner) }, std::forward<T>(value) };
        }

        template <typename Sender, typename T>
            requires tag_invocable<context_set_t, Sender, T>
        auto operator()(Sender &&sender, T &&value) const
            noexcept(is_nothrow_tag_invocable_v<context_set_t, Sender, T>)
                -> tag_invoke_result_t<context_set_t, Sender, T>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), std::forward<T>(value));
        }

        template <typename T>
        auto operator()(T &&value) const
        {
            return pipable_from_right(*this, std::forward<T>(value));
        }
    };

    inline constexpr context_set_t context_set;

}

namespace Serialize {
    template <typename T>
    struct Operations<Behavior::ContextParameter<T>> {
        template <typename Context>
        static StreamResult read(FormattedSerializeStream &in, Behavior::ContextParameter<T> &n, const char *name, Context &&context)
        {
            return Serialize::read(in, n.mValue, name, context);
        }
        template <typename Context>
        static void write(FormattedSerializeStream &out, const Behavior::ContextParameter<T> &n, const char *name, Context &&context)
        {
            Serialize::write(out, n.mValue, name, context);
        }
        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth)
        {
            Serialize::visitStream<std::optional<forward_ref_t<T>>>(in, name, visitor, depth);
        }
    };
}

}
