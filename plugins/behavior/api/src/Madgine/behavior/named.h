#pragma once

#include "Generic/callable_view.h"
#include "Generic/execution/concepts.h"
#include "Generic/execution/storage.h"
#include "Generic/fixed_string.h"

#include "Meta/reflect/util.h"
#include "Meta/serialize/operations.h"
#include "Meta/serialize/streams/streamresult.h"

#include "named_d.h"

namespace Engine {
namespace Behavior {

    template <fixed_string Name, typename V>
    struct get_named_t {

        template <typename T, typename O>
            requires(!is_tag_invocable_v<get_named_t, T, O>)
        Reflect::Result operator()(T &&context, O &o) const
        {
            Reflect::Result result;
            auto f = [&](Reflect::Value &v) {
                result = get_named_d(std::forward<T>(context), Name, v);
                if (!result)
                    result = invoke_free(v, [&](const V &v) { o = v; }, v);
            };
            Value_erased(CallableView<void(Reflect::Value &)> { f });
            return result;
        }

        template <typename T, typename O>
            requires(is_tag_invocable_v<get_named_t, T, O>)
        auto operator()(T &&context, O &&o) const
            noexcept(is_nothrow_tag_invocable_v<get_named_t, T, O>)
                -> tag_invoke_result_t<get_named_t, T, O>
        {
            return tag_invoke(*this, std::forward<T>(context), std::forward<O>(o));
        }
    };

    template <fixed_string Name, typename V>
    inline constexpr get_named_t<Name, V> get_named;

    template <typename Rec, fixed_string Name, typename T>
    struct NamedState : Execution::base_state<Rec> {

        NamedState(Named<Name, T> value, Rec &&rec)
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

        friend auto tag_invoke(Execution::visit_state_t, NamedState *state, auto &&visitor)
        {
        }

        Named<Name, T> mValue;
    };

    template <fixed_string Name, typename T>
    struct Named {

        Named() = default;

        Named(T &&value)
            : mValue(std::in_place, std::forward<T>(value))
        {
        }

        Named(std::optional<forward_ref_t<T>> value)
            : mValue(std::move(value))
        {
        }

        using is_sender = void;

        using result_type = Reflect::Error;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<T &>;

        Reflect::Result resolve(auto &context)
        {
            if (!mValue) {
                return get_named<Name, T>(context, mValue);
            }
            return {};
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
        friend auto tag_invoke(Execution::connect_t, Named &&sender, Rec &&rec)
        {
            return NamedState<Rec, Name, T> { std::move(sender), std::forward<Rec>(rec) };
        }

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, Named &sender, Rec &&rec)
        {
            return NamedState<Rec, Name, T> { sender, std::forward<Rec>(rec) };
        }

        using meta_t = std::optional<forward_ref_t<T>>;

        template <bool isReferenceWrapped>
        friend decltype(auto) tag_invoke(Reflect::convert_Value_t<isReferenceWrapped> convert_ValueType, Named<Name, T> &named)
        {
            return convert_ValueType(named.mValue);
        }

        template <bool isReferenceWrapped>
        friend decltype(auto) tag_invoke(Reflect::convert_Value_t<isReferenceWrapped> convert_ValueType, Named<Name, T> &&named)
        {
            return convert_ValueType(std::move(named.mValue));
        }

        friend Serialize::StreamResult tag_invoke(Serialize::apply_map_t, Named<Name, T> &named, Serialize::CallerHierarchyFormattedSerializeStream in, bool success)
        {
            if (named.mValue)
                return Serialize::apply_map(*named.mValue, in, success);
            else
                return {};
        }

        std::optional<forward_ref_t<T>> mValue;
    };

    template <fixed_string Name>
    struct with_named_t {

        template <typename Rec, typename T>
        struct receiver : Execution::algorithm_receiver<Rec> {

            receiver(Rec &&rec, T &&value)
                : Execution::algorithm_receiver<Rec> { std::forward<Rec>(rec) }
                , mValue(std::forward<T>(value))
            {
            }

            friend Reflect::Result tag_invoke(get_named_d_t, receiver &rec, std::string_view name, Reflect::ValueRef out)
            {
                if (name == Name) {
                    toValue(out, rec.mValue);
                    return {};
                } else {
                    return get_named_d(rec.mRec, name, out);
                }
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
        friend auto tag_invoke(with_named_t, Sender &&inner, T &&value)
        {
            return sender<Sender, T> { { {}, std::forward<Sender>(inner) }, std::forward<T>(value) };
        }

        template <typename Sender, typename T>
            requires tag_invocable<with_named_t, Sender, T>
        auto operator()(Sender &&sender, T &&value) const
            noexcept(is_nothrow_tag_invocable_v<with_named_t, Sender, T>)
                -> tag_invoke_result_t<with_named_t, Sender, T>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), std::forward<T>(value));
        }

        template <typename T>
        auto operator()(T &&value) const
        {
            return pipable_from_right(*this, std::forward<T>(value));
        }
    };

    template <fixed_string Name>
    constexpr with_named_t<Name> with_named;

}

namespace Serialize {
    template <fixed_string Name, typename T>
    struct Operations<Behavior::Named<Name, T>> {
        static StreamResult read(CallerHierarchyFormattedSerializeStream in, Behavior::Named<Name, T> &n, const char *name)
        {
            return Serialize::read(in, n.mValue, name);
        }
        static void write(CallerHierarchyFormattedSerializeStream out, const Behavior::Named<Name, T> &n, const char *name)
        {
            Serialize::write(out, n.mValue, name);
        }
        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
        {
            Serialize::visitStream<std::optional<forward_ref_t<T>>>(in, name, visitor, depth);
        }
    };
}

}
