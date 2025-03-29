#pragma once

#include "../delayedconstruct.h"
#include "../manuallifetime.h"
#include "../pipable.h"
#include "concepts.h"
#include "storage.h"

#undef ERROR

namespace Engine {
namespace Execution {

    struct just_t {

        template <typename Rec, typename... Args>
        struct state : base_state<Rec> {

            void start()
            {
                TupleUnpacker::invokeFromTuple(LIFT(this->mRec.set_value, this), mArgs);
            }

            std::tuple<Args...> mArgs;
        };

        template <typename... Args>
        struct sender {
            using result_type = void;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<Args...>;

            using is_sender = void;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Rec, Args...> { std::forward<Rec>(rec), std::move(sender.mArgs) };
            }

            std::tuple<Args...> mArgs;
        };

        template <typename... Args>
        auto operator()(Args &&...args) const
        {
            return sender<std::decay_t<Args>...> { { std::forward<Args>(args)... } };
        }
    };

    inline constexpr just_t just;

    struct just_error_t {

        template <typename Rec, typename R>
        struct state : base_state<Rec> {

            void start()
            {
                this->mRec.set_error(mError);
            }

            R mError;
        };

        template <typename R>
        struct sender {
            using result_type = R;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<>;

            using is_sender = void;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Rec, R> { std::forward<Rec>(rec), std::move(sender.mError) };
            }

            R mError;
        };

        template <typename R>
        auto operator()(R &&error) const
        {
            return sender<R> { std::forward<R>(error) };
        }
    };

    inline constexpr just_error_t just_error;

    struct then_t {

        template <typename Rec, typename T>
        struct receiver : algorithm_receiver<Rec> {

            receiver(Rec &&rec, T &&transform)
                : algorithm_receiver<Rec> { std::forward<Rec>(rec) }
                , mTransform(std::forward<T>(transform))
            {
            }

            template <typename... V>
            void set_value(V &&...values)
            {
                using V2 = decltype(TupleUnpacker::invoke(mTransform, std::forward<V>(values)...));
                if constexpr (std::same_as<V2, void>) {
                    TupleUnpacker::invoke(mTransform, std::forward<V>(values)...);
                    this->mRec.set_value();
                } else if constexpr (InstanceOf<V2, std::tuple>)
                    TupleUnpacker::invokeExpand(LIFT(this->mRec.set_value, this), TupleUnpacker::invoke(mTransform, std::forward<V>(values)...));
                else
                    this->mRec.set_value(TupleUnpacker::invoke(mTransform, std::forward<V>(values)...));
            }

            T mTransform;
        };

        template <typename T>
        static auto return_types_helper()
        {
            if constexpr (std::same_as<T, void>) {
                return type_pack<> {};
            } else if constexpr (InstanceOf<T, std::tuple>) {
                return to_type_pack<T> {};
            } else {
                return type_pack<T> {};
            }
        }

        template <Sender Sender, typename T>
        struct sender : algorithm_sender<Sender> {

            template <typename... V>
            using helper = decltype(TupleUnpacker::invoke(std::declval<T>(), std::declval<V>()...));
            using helper_type = typename std::decay_t<Sender>::template value_types<helper>;
            template <template <typename...> typename Tuple>
            using value_types = typename decltype(return_types_helper<helper_type>())::template instantiate<Tuple>;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return algorithm_state<Sender, receiver<Rec, T>> { std::forward<Sender>(sender.mSender), std::forward<Rec>(rec), std::forward<T>(sender.mTransform) };
            }

            T mTransform;
        };

        template <Sender Sender, typename T>
        friend auto tag_invoke(then_t, Sender &&inner, T &&transform)
        {
            return sender<Sender, T> { { {}, std::forward<Sender>(inner) }, std::forward<T>(transform) };
        }

        template <typename Sender, typename T>
            requires tag_invocable<then_t, Sender, T>
        auto operator()(Sender &&sender, T &&transform) const
            noexcept(is_nothrow_tag_invocable_v<then_t, Sender, T>)
                -> tag_invoke_result_t<then_t, Sender, T>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), std::forward<T>(transform));
        }

        template <typename T>
        auto operator()(T &&transform) const
        {
            return pipable_from_right(*this, std::forward<T>(transform));
        }

        template <typename T>
        struct typed {
            template <typename Sender>
                requires tag_invocable<then_t, Sender, T>
            auto operator()(Sender &&sender, T &&transform = {}) const
                noexcept(is_nothrow_tag_invocable_v<then_t, Sender, T>)
                    -> tag_invoke_result_t<then_t, Sender, T>
            {
                return tag_invoke(then_t {}, std::forward<Sender>(sender), std::forward<T>(transform));
            }

            auto operator()(T &&transform = {}) const
            {
                return pipable_from_right(then_t {}, std::forward<T>(transform));
            }
        };
    };

    inline constexpr then_t then;

    struct after_t {

        template <typename Inner, typename _Rec, typename T>
        struct state {

            using Rec = _Rec;

            using State = connect_result_t<Inner, Rec>;

            state(Inner &&inner, Rec &&rec, T &&func)
                : mFunc(std::forward<T>(func))
                , mState(connect(std::forward<Inner>(inner), std::forward<Rec>(rec)))
            {
            }

            void start()
            {
                mFunc();
                mState.start();
            }

            T mFunc;
            State mState;
        };

        template <Sender Sender, typename T>
        struct sender : algorithm_sender<Sender> {

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Sender, Rec, T> { std::forward<Sender>(sender.mSender), std::forward<Rec>(rec), std::forward<T>(sender.mFunc) };
            }

            T mFunc;
        };

        template <Sender Sender, typename T>
        friend auto tag_invoke(after_t, Sender &&inner, T &&func)
        {
            return sender<Sender, T> { { {}, std::forward<Sender>(inner) }, std::forward<T>(func) };
        }

        template <typename Sender, typename T>
            requires tag_invocable<after_t, Sender, T>
        auto operator()(Sender &&sender, T &&func) const
            noexcept(is_nothrow_tag_invocable_v<after_t, Sender, T>)
                -> tag_invoke_result_t<after_t, Sender, T>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), std::forward<T>(func));
        }

        template <typename T>
        auto operator()(T &&func) const
        {
            return pipable_from_right(*this, std::forward<T>(func));
        }

        template <typename T>
        struct typed {
            template <typename Sender>
                requires tag_invocable<after_t, Sender, T>
            auto operator()(Sender &&sender, T &&func = {}) const
                noexcept(is_nothrow_tag_invocable_v<after_t, Sender, T>)
                    -> tag_invoke_result_t<after_t, Sender, T>
            {
                return tag_invoke(after_t {}, std::forward<Sender>(sender), std::forward<T>(func));
            }

            auto operator()(T &&func = {}) const
            {
                return pipable_from_right(after_t {}, std::forward<T>(func));
            }
        };
    };

    inline constexpr after_t after;

    struct onError_t {

        template <typename Rec, typename T>
        struct receiver : algorithm_receiver<Rec> {

            template <typename... R>
            void set_error(R &&...error)
            {
                TupleUnpacker::invoke(mOnError, std::forward<R>(error)...);
                this->mRec.set_error(std::forward<R>(error)...);
            }

            T mOnError;
        };

        template <typename Sender, typename T>
        struct sender : algorithm_sender<Sender> {

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return algorithm_state<Sender, receiver<Rec, T>> { std::forward<Sender>(sender.mSender), std::forward<Rec>(rec), std::forward<T>(sender.mOnError) };
            }

            T mOnError;
        };

        template <typename Sender, typename T>
        friend auto tag_invoke(onError_t, Sender &&inner, T &&onError)
        {
            return sender<Sender, T> { { {}, std::forward<Sender>(inner) }, std::forward<T>(onError) };
        }

        template <typename Sender, typename T>
            requires tag_invocable<onError_t, Sender, T>
        auto operator()(Sender &&sender, T &&onError) const
            noexcept(is_nothrow_tag_invocable_v<onError_t, Sender, T>)
                -> tag_invoke_result_t<onError_t, Sender, T>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), std::forward<T>(onError));
        }

        template <typename T>
        auto operator()(T &&onError) const
        {
            return pipable_from_right(*this, std::forward<T>(onError));
        }
    };

    inline constexpr onError_t onError;

    struct reduce_stream_t {

        template <typename Rec, typename Stream, typename T, typename F>
        struct state : base_state<Rec> {

            struct receiver {
                template <typename CPO, typename... Args>
                friend auto tag_invoke(CPO f, receiver &rec, Args &&...args)
                    -> tag_invoke_result_t<CPO, Rec &, Args...>
                {
                    return f(rec.mState->mRec, std::forward<Args>(args)...);
                }

                template <typename U>
                void set_value(U &&value)
                {
                    mState->reduce(std::forward<U>(value));
                }

                template <typename... R>
                void set_error(R &&...error)
                {
                    throw 0;
                }

                void set_done()
                {
                    mState->done();
                }

                state *mState;
            };

            using Sender = std::invoke_result_t<decltype(&Stream::next), Stream>;

            using State = connect_result_t<Sender, receiver>;

            void start()
            {
                construct(mState,
                    DelayedConstruct<State> { [this]() { return connect(mStream.next(), receiver { this }); } });
                mState->start();
            }

            void done()
            {
                destruct(mState);
                this->mRec.set_value(std::forward<T>(mAcc));
            }

            template <typename U>
            void reduce(U &&value)
            {
                mAcc = mReducer(std::forward<T>(mAcc), std::forward<U>(value));
                destruct(mState);
                start();
            }

            Stream mStream;
            T mAcc;
            F mReducer;

            ManualLifetime<State> mState;
        };

        template <typename Stream, typename T, typename F>
        struct sender : base_sender {
            using result_type = void;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<T>;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Rec, Stream, T, F> { std::forward<Rec>(rec), std::forward<Stream>(sender.mStream), std::forward<T>(sender.mInitial), std::forward<F>(sender.mReducer) };
            }

            Stream mStream;
            T mInitial;
            F mReducer;
        };

        template <typename Stream, typename T, typename F>
        friend auto tag_invoke(reduce_stream_t, Stream &&stream, T &&initial, F &&reducer)
        {
            return sender<Stream, T, F> { {}, std::forward<Stream>(stream), std::forward<T>(initial), std::forward<F>(reducer) };
        }

        template <typename Stream, typename T, typename F>
            requires tag_invocable<reduce_stream_t, Stream, T, F>
        auto operator()(Stream &&stream, T &&initial, F &&reducer) const
            noexcept(is_nothrow_tag_invocable_v<reduce_stream_t, Stream, T, F>)
                -> tag_invoke_result_t<reduce_stream_t, Stream, T, F>
        {
            return tag_invoke(*this, std::forward<Stream>(stream), std::forward<T>(initial), std::forward<F>(reducer));
        }

        template <typename T, typename F>
        auto operator()(T &&initial, F &&reducer) const
        {
            return pipable_from_right(*this, std::forward<T>(initial), std::forward<F>(reducer));
        }
    };

    inline constexpr reduce_stream_t reduce_stream;

    struct finally_t {

        template <typename Rec, typename F>
        struct receiver : algorithm_receiver<Rec> {

            receiver(Rec &&rec, F &&f)
                : algorithm_receiver<Rec>(std::forward<Rec>(rec))
                , mFinally(std::forward<F>(f))
            {
            }

            template <typename... V>
            void set_value(V &&...values)
            {
                mFinally();
                this->mRec.set_value(std::forward<V>(values)...);
            }

            template <typename... R>
            void set_error(R &&...errors)
            {
                mFinally();
                this->mRec.set_error(std::forward<R>(errors)...);
            }

            void set_done()
            {
                mFinally();
                this->mRec.set_done();
            }

            F mFinally;
        };

        template <typename Sender, typename F>
        struct sender : algorithm_sender<Sender> {

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return algorithm_state<Sender, receiver<Rec, F>> { std::forward<Sender>(sender.mSender), std::forward<Rec>(rec), std::forward<F>(sender.mFinally) };
            }

            F mFinally;
        };

        template <typename Sender, typename F>
        friend auto tag_invoke(finally_t, Sender &&inner, F &&finally)
        {
            return sender<Sender, F> { { {}, std::forward<Sender>(inner) }, std::forward<F>(finally) };
        }

        template <typename Sender, typename F>
            requires tag_invocable<finally_t, Sender, F>
        auto operator()(Sender &&sender, F &&finally) const
            noexcept(is_nothrow_tag_invocable_v<finally_t, Sender, F>)
                -> tag_invoke_result_t<finally_t, Sender, F>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), std::forward<F>(finally));
        }

        template <typename F>
        auto operator()(F &&finally) const
        {
            return pipable_from_right(*this, std::forward<F>(finally));
        }

        template <typename F>
        struct typed {
            template <typename Sender>
                requires tag_invocable<finally_t, Sender, F>
            auto operator()(Sender &&sender, F &&finally = {}) const
                noexcept(is_nothrow_tag_invocable_v<finally_t, Sender, F>)
                    -> tag_invoke_result_t<finally_t, Sender, F>
            {
                return tag_invoke(finally_t {}, std::forward<Sender>(sender), std::forward<F>(finally));
            }

            auto operator()(F &&finally = {}) const
            {
                return pipable_from_right(finally_t {}, std::forward<F>(finally));
            }
        };
    };

    inline constexpr finally_t finally;

    template <typename F>
    inline constexpr finally_t::typed<F> typed_finally;

    struct for_each_t {

        template <typename Rec, typename C, typename F>
        struct state;

        template <typename Rec, typename C, typename F>
        struct receiver : algorithm_receiver<Rec &> {

            receiver(Rec &rec, state<Rec, C, F> *state)
                : algorithm_receiver<Rec &>(rec)
                , mState(state)
            {
            }

            template <typename... V>
            void set_value(V &&...value)
            {
                mState->set_value(this->mRec, std::forward<V>(value)...);
            }

            void set_done()
            {
                mState->set_done(this->mRec);
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mState->set_error(this->mRec, std::forward<R>(result)...);
            }

            state<Rec, C, F> *mState;
        };

        template <typename Rec, typename C, typename F>
        struct state : base_state<Rec> {
            using value_type = typename std::remove_reference_t<C>::value_type &;
            using inner_sender_t = std::invoke_result_t<F, value_type>;
            using inner_state_t = connect_result_t<inner_sender_t, receiver<Rec, C, F>>;

            state(Rec &&rec, C &&c, F &&f)
                : base_state<Rec>(std::forward<Rec>(rec))
                , mF(std::forward<F>(f))
                , mC(std::forward<C>(c))
            {
            }

            ~state() { }

            void start()
            {
                mIt = mC.begin();
                loop(this->mRec);
            }

            void stopInner()
            {
                destruct(mState);
            }

            template <typename... Args>
            void set_value(Rec &rec, Args &&...)
            {
                stopInner();
                if (mFlag.test_and_set())
                    loop(rec);
            }

            void set_done(Rec &rec)
            {
                stopInner();
                rec.set_done();
            }

            template <typename... R>
            void set_error(Rec &rec, R... result)
            {
                stopInner();
                rec.set_error(result...);
            }

            void loop(Rec &rec)
            {
                while (proceed(rec))
                    ++mIt;
            }

            bool proceed(Rec &rec)
            {
                if (mIt == mC.end()) {
                    rec.set_value();
                    return false;
                } else {
                    mFlag.clear();
                    construct(mState,
                        DelayedConstruct<inner_state_t> {
                            [&]() {
                                return connect(mF(*mIt), receiver<Rec, C, F> { rec, this });
                            } });
                    mState->start();
                    return mFlag.test_and_set();
                }
            }

            F mF;
            using iterator = typename std::remove_reference_t<C>::iterator;
            C mC;
            iterator mIt;
            std::atomic_flag mFlag;
            ManualLifetime<inner_state_t> mState;
        };

        template <typename C, typename F>
        struct sender : base_sender {
            using value_type = typename std::remove_reference_t<C>::value_type &;
            using inner_sender_t = std::invoke_result_t<F, value_type>;
            using result_type = typename inner_sender_t::result_type;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<>;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Rec, C, F> { std::forward<Rec>(rec), std::forward<C>(sender.mContainer), std::forward<F>(sender.mF) };
            }

            C mContainer;
            F mF;
        };

        template <typename C, typename F>
        friend auto tag_invoke(for_each_t, C &&c, F &&f)
        {
            return sender<C, F> { {}, std::forward<C>(c), std::forward<F>(f) };
        }

        template <typename C, typename F>
            requires tag_invocable<for_each_t, C, F>
        auto operator()(C &&container, F &&f) const
            noexcept(is_nothrow_tag_invocable_v<for_each_t, C, F>)
                -> tag_invoke_result_t<for_each_t, C, F>
        {
            return tag_invoke(*this, std::forward<C>(container), std::forward<F>(f));
        }
    };

    inline constexpr for_each_t for_each;

    struct repeat_t {

        template <typename Rec, typename Sender>
        struct state;

        template <typename Rec, typename Sender>
        struct receiver : algorithm_receiver<Rec> {

            template <typename... V>
            void set_value(V &&...value)
            {
                mState->set_value(this->mRec, std::forward<V>(value)...);
            }

            void set_done()
            {
                mState->set_done(this->mRec);
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mState->set_error(this->mRec, std::forward<R>(result)...);
            }

            state<Rec, Sender> *mState;
        };

        template <typename Rec, typename Sender>
        struct state : algorithm_state<Sender, receiver<Rec, Sender>> {
            using base = algorithm_state<Sender, receiver<Rec, Sender>>;

            state(Rec &&rec, Sender &&sender)
                : base(std::forward<Sender>(sender), std::forward<Rec>(rec), this)
            {
            }

            ~state() { }

            template <typename... V>
            void set_value(Rec &, V &&...args)
            {
                this->start();
            }
        };

        template <typename Sender>
        struct sender : algorithm_sender<Sender> {

            template <template <typename...> typename Tuple>
            using value_types = Tuple<>;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Rec, Sender> { std::forward<Rec>(rec), std::forward<Sender>(sender.mSender) };
            }
        };

        template <typename Sender>
        friend auto tag_invoke(repeat_t, Sender &&inner)
        {
            return sender<Sender> { { {}, std::forward<Sender>(inner) } };
        }

        template <typename Sender>
            requires tag_invocable<repeat_t, Sender>
        auto operator()(Sender &&sender) const
            noexcept(is_nothrow_tag_invocable_v<repeat_t, Sender>)
                -> tag_invoke_result_t<repeat_t, Sender>
        {
            return tag_invoke(*this, std::forward<Sender>(sender));
        }

        template <typename Sender>
        friend auto operator|(Sender &&sender, repeat_t r)
        {
            return tag_invoke(r, std::forward<Sender>(sender));
        }
    };

    inline constexpr repeat_t repeat;

    struct when_all_t {

        template <typename Rec, typename Is, typename... Sender>
        struct state;

        template <size_t I, typename Rec, typename Is, typename... Sender>
        struct receiver {

            template <typename... V>
            void set_value(V &&...value)
            {
                mState->template set_value<I>(std::forward<V>(value)...);
            }

            void set_done()
            {
                mState->set_done();
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mState->set_error(std::forward<R>(result)...);
            }

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, receiver &rec, Args &&...args)
                -> tag_invoke_result_t<CPO, Rec &, Args...>
            {
                return f(rec.mState->mRec, std::forward<Args>(args)...);
            }

            state<Rec, Is, Sender...> *mState;
        };

        template <size_t I, typename Sender, typename Rec, typename Is, typename... Senders>
        using inner_state = connect_result_t<Sender, receiver<I, Rec, Is, Senders...>>;

        template <typename Rec, size_t... Is, typename... Sender>
        struct state<Rec, std::index_sequence<Is...>, Sender...> : base_state<Rec> {

            using Tuple = std::tuple<typename Sender::template value_types<std::tuple>...>;
            using R = typename first_t<Sender...>::result_type;

            state(Rec &&rec, Sender &&...senders)
                : base_state<Rec> { std::forward<Rec>(rec) }
                , mStates {
                    DelayedConstruct<inner_state<Is, Sender, Rec, std::index_sequence<Is...>, Sender...>> {
                        [&]() { return connect(std::forward<Sender>(senders), receiver<Is, Rec, std::index_sequence<Is...>, Sender...> { this }); } }...
                }
            {
            }

            ~state() { }

            void start()
            {
                mCompleteCount = 0;
                mState = OK;
                TupleUnpacker::forEach(mStates, [](auto &state) { state.start(); });
                mark_complete();
            }

            template <size_t I, typename... V>
            void set_value(V &&...values)
            {
                if (mState == OK)
                    std::get<I>(mValues) = { std::forward<V>(values)... };
                mark_complete();
            }

            void set_done()
            {
                mState = DONE;
                mark_complete();
            }

            void set_error(patch_void_t<R> result)
            {
                State expected = OK;
                if (mState.compare_exchange_strong(expected, ERROR)) {
                    mResult = result;
                }
                mark_complete();
            }

            void mark_complete()
            {
                size_t count = mCompleteCount.fetch_add(1);
                if (count == sizeof...(Is)) {
                    switch (mState) {
                    case OK:
                        TupleUnpacker::invokeFromTuple(LIFT(this->mRec.set_value, &), Tuple { std::move(mValues) });
                        break;
                    case ERROR:
                        this->mRec.set_error(std::move(mResult));
                        break;
                    case DONE:
                        this->mRec.set_done();
                        break;
                    default:
                        throw 0;
                    }
                }
            }

            std::tuple<inner_state<Is, Sender, Rec, std::index_sequence<Is...>, Sender...>...> mStates;
            patch_void_t<R> mResult;
            template <typename... V>
            using helper = std::tuple<std::conditional_t<std::is_reference_v<V>, OutRef<std::remove_reference_t<V>>, V>...>;
            std::tuple<typename Sender::template value_types<helper>...> mValues;
            enum State { OK,
                ERROR,
                DONE };
            std::atomic<State> mState = OK;
            std::atomic<size_t> mCompleteCount = 0;
        };

        template <typename... Sender>
        struct sender : base_sender {

            using result_type = typename first_t<Sender...>::result_type;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<typename Sender::template value_types<std::tuple>...>;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return TupleUnpacker::constructExpand<state<Rec, std::index_sequence_for<Sender...>, Sender...>>(std::forward<Rec>(rec), std::move(sender.mSenders));
            };

            std::tuple<Sender...> mSenders;
        };

        template <typename... Sender>
        friend auto tag_invoke(when_all_t, Sender &&...senders)
        {
            return sender<Sender...> { {}, { std::forward<Sender>(senders)... } };
        }

        template <typename... S>
            requires tag_invocable<when_all_t, S...>
        auto operator()(S &&...senders) const
            noexcept(is_nothrow_tag_invocable_v<when_all_t, S...>)
                -> tag_invoke_result_t<when_all_t, S...>
        {
            return tag_invoke(*this, std::forward<S>(senders)...);
        }
    };

    inline constexpr when_all_t when_all;

    struct when_all_range_t {

        template <typename Rec, typename Sender>
        struct state;

        template <typename Rec, typename Sender>
        struct receiver {

            template <typename... V>
            void set_value(V &&...value)
            {
                mState->set_value(mIndex, std::forward<V>(value)...);
            }

            void set_done()
            {
                mState->set_done();
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mState->set_error(std::forward<R>(result)...);
            }

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, receiver &rec, Args &&...args)
                -> tag_invoke_result_t<CPO, Rec &, Args...>
            {
                return f(rec.mState->mRec, std::forward<Args>(args)...);
            }

            size_t mIndex;
            state<Rec, Sender> *mState;
        };

        template <typename Rec, typename Sender>
        struct state : base_state<Rec> {

            using inner_state = connect_result_t<Sender, receiver<Rec, Sender>>;

            using Values = std::vector<typename Sender::template value_types<identity>>;
            using R = typename Sender::result_type;

            template <typename R>
            state(Rec &&rec, R &&range)
                : base_state<Rec> { std::forward<Rec>(rec) }
                , mStates(std::make_unique<ManualLifetime<inner_state>[]>(range.size()))
                , mValues(range.size())
                , mSize(range.size())
            {
                size_t i = 0;
                for (auto &&sender : range) {
                    construct(mStates[i], DelayedConstruct<inner_state> { [&, this, i]() { return connect(std::forward<decltype(sender)>(sender), receiver<Rec, Sender> { i, this }); } });
                    ++i;
                }
            }

            ~state()
            {
                std::ranges::for_each(std::span { mStates.get(), mSize }, destruct);
            }

            void start()
            {
                mCompleteCount = 0;
                mState = OK;
                std::ranges::for_each(std::span { mStates.get(), mSize }, [](auto &state) { state->start(); });
                mark_complete();
            }

            template <typename V>
            void set_value(size_t index, V &&value)
            {
                if (mState == OK)
                    mValues[index] = std::forward<V>(value);
                mark_complete();
            }

            void set_done()
            {
                mState = DONE;
                mark_complete();
            }

            void set_error(patch_void_t<R> result)
            {
                State expected = OK;
                if (mState.compare_exchange_strong(expected, ERROR)) {
                    mResult = std::move(result);
                }
                mark_complete();
            }

            void mark_complete()
            {
                size_t count = mCompleteCount.fetch_add(1);
                if (count == mSize) {
                    switch (mState) {
                    case OK:
                        this->mRec.set_value(std::move(mValues));
                        break;
                    case ERROR:
                        this->mRec.set_error(std::move(mResult));
                        break;
                    case DONE:
                        this->mRec.set_done();
                        break;
                    default:
                        throw 0;
                    }
                }
            }

            std::unique_ptr<ManualLifetime<inner_state>[]> mStates;
            patch_void_t<R> mResult;
            Values mValues;
            size_t mSize;
            enum State { OK,
                ERROR,
                DONE };
            std::atomic<State> mState = OK;
            std::atomic<size_t> mCompleteCount = 0;
        };

        template <typename R>
        struct sender : base_sender {

            using result_type = typename R::value_type::result_type;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<std::vector<typename R::value_type::template value_types<identity>>>;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Rec, typename R::value_type>(std::forward<Rec>(rec), std::forward<R>(sender.mRange));
            };

            R mRange;
        };

        template <typename R>
        friend auto tag_invoke(when_all_range_t, R &&range)
        {
            return sender<R> { {}, std::forward<R>(range) };
        }

        template <typename R>
            requires tag_invocable<when_all_range_t, R>
        auto operator()(R &&range) const
            noexcept(is_nothrow_tag_invocable_v<when_all_range_t, R>)
                -> tag_invoke_result_t<when_all_range_t, R>
        {
            return tag_invoke(*this, std::forward<R>(range));
        }
    };

    inline constexpr when_all_range_t when_all_range;

    struct sequence_t {

        template <size_t Is>
        struct inner_tag {
        };

        template <typename Rec, typename... Sender>
        struct state;

        template <typename Tag, typename Rec, typename... Sender>
        struct receiver {

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, receiver &rec, Args &&...args)
                -> tag_invoke_result_t<CPO, Rec &, Args...>
            {
                return f(rec.mState->mRec, std::forward<Args>(args)...);
            }

            template <typename... V>
            void set_value(V &&...value)
            {
                mState->set_value(Tag {}, std::forward<V>(value)...);
            }

            void set_done()
            {
                mState->set_done(Tag {});
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mState->set_error(Tag {}, std::forward<R>(result)...);
            }

            state<Rec, Sender...> *mState;
        };

        template <typename Rec, typename... Sender>
        struct state : base_state<Rec> {

            template <size_t I>
            using inner_rec = receiver<inner_tag<I>, Rec, Sender...>;

            template <size_t... Is>
            static auto stateTupleHelper(std::index_sequence<Is...>) -> std::variant<std::monostate, connect_result_t<Sender, inner_rec<Is>>...> { }
            using StateVariant = decltype(stateTupleHelper(std::index_sequence_for<Sender...> {}));

            state(Rec &&rec, std::tuple<Sender...> &&senders)
                : base_state<Rec> { std::forward<Rec>(rec) }
                , mSenders(std::move(senders))
            {
            }

            ~state() { }

            void start()
            {
                if constexpr (sizeof...(Sender) == 0)
                    this->mRec.set_value();
                else {
                    mStates.template emplace<1>(
                               DelayedConstruct<std::variant_alternative_t<1, StateVariant>> {
                                   [this]() {
                                       return Execution::connect(std::forward<std::tuple_element_t<0, std::tuple<Sender...>>>(std::get<0>(mSenders)), inner_rec<0> { this });
                                   } })
                        .start();
                }
            }

            template <size_t I, typename... V>
            void set_value(inner_tag<I>, V &&...values)
            {
                if constexpr (sizeof...(Sender) == I + 1)
                    this->mRec.set_value();
                else {
                    mStates.template emplace<I + 1 + 1>(
                               DelayedConstruct<std::variant_alternative_t<I + 1 + 1, StateVariant>> {
                                   [this]() {
                                       return Execution::connect(std::forward<std::tuple_element_t<I + 1, std::tuple<Sender...>>>(std::get<I + 1>(mSenders)), inner_rec<I + 1> { this });
                                   } })
                        .start();
                }
            }

            template <size_t I>
            void set_done(inner_tag<I>)
            {
                this->mRec.set_done();
            }

            template <size_t I, typename... R>
            void set_error(inner_tag<I>, R &&...result)
            {
                this->mRec.set_error(std::forward<R>(result)...);
            }

            std::tuple<Sender...> mSenders;

            StateVariant mStates;
        };

        template <typename... Sender>
        struct sender : base_sender {
            using result_type = typename std::remove_reference_t<first_t<Sender...>>::result_type;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<>;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Rec, Sender...> { std::forward<Rec>(rec), std::move(sender.mSenders) };
            }

            std::tuple<Sender...> mSenders;
        };

        template <typename Sender>
        friend decltype(auto) tag_invoke(sequence_t, Sender &&sender)
        {
            return std::forward<Sender>(sender);
        }

        template <typename... Sender>
        friend auto tag_invoke(sequence_t sequence, Sender &&...senders)
        {
            return sender<Sender...> { {}, { std::forward<Sender>(senders)... } };
        }

        template <typename... S>
            requires tag_invocable<sequence_t, S...>
        auto operator()(S &&...senders) const
            noexcept(is_nothrow_tag_invocable_v<sequence_t, S...>)
                -> tag_invoke_result_t<sequence_t, S...>
        {
            return tag_invoke(*this, std::forward<S>(senders)...);
        }
    };

    inline constexpr sequence_t sequence;

    /* template <Sender S1, Sender S2>
    requires tag_invocable<sequence_t, S1, S2>
    auto operator|(S1 &&sender1, S2 &&sender2)
    {
        return sequence(std::forward<S1>(sender1), std::forward<S2>(sender2));
    }*/

    struct let_value_t {

        template <typename Rec, typename Sender, typename F>
        struct state;

        template <typename Rec, typename Sender, typename F, typename QualifiedRec, typename... Tags>
        struct receiver : algorithm_receiver<QualifiedRec> {

            receiver(QualifiedRec &&rec, state<Rec, Sender, F> *state)
                : algorithm_receiver<QualifiedRec>(std::forward<QualifiedRec>(rec))
                , mState(state)
            {
            }

            template <typename... V>
            void set_value(V &&...value)
            {
                mState->set_value(Tags {}..., this->mRec, std::forward<V>(value)...);
            }

            void set_done()
            {
                mState->set_done(Tags {}..., this->mRec);
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mState->set_error(Tags {}..., this->mRec, std::forward<R>(result)...);
            }

            state<Rec, Sender, F> *mState;
        };

        template <typename Rec, typename Sender, typename F>
        struct state : algorithm_state<Sender, receiver<Rec, Sender, F, Rec>> {
            using base = algorithm_state<Sender, receiver<Rec, Sender, F, Rec>>;

            struct inner_tag {
            };
            template <typename... V>
            using helper = std::invoke_result_t<F, V &...>;
            using inner_sender_t = typename Sender::template value_types<helper>;
            using Tuple = typename Sender::template value_types<std::tuple>;
            using inner_state = connect_result_t<inner_sender_t, receiver<Rec, Sender, F, Rec &, inner_tag>>;

            state(Rec &&rec, Sender &&sender, F &&f)
                : algorithm_state<Sender, receiver<Rec, Sender, F, Rec>> { std::forward<Sender>(sender), std::forward<Rec>(rec), this }
                , mF(std::forward<F>(f))
            {
            }

            ~state() { }

            template <typename... V>
            void set_value(Rec &rec, V &&...values)
            {
                construct(mValue, std::forward<V>(values)...);
                construct(mInnerState,
                    DelayedConstruct<inner_state> {
                        [&]() { return connect(TupleUnpacker::invokeFromTuple(mF, static_cast<Tuple &>(mValue)), receiver<Rec, Sender, F, Rec &, inner_tag> { rec, this }); } });
                mInnerState->start();
            }
            using base::set_done;
            using base::set_error;

            void cleanup()
            {
                destruct(mInnerState);
                destruct(mValue);
            }

            template <typename... V2>
            void set_value(inner_tag, Rec &rec, V2 &&...values)
            {
                cleanup();
                rec.set_value(std::forward<V2>(values)...);
            }

            void set_done(inner_tag, Rec &rec)
            {
                cleanup();
                rec.set_done();
            }

            template <typename... R>
            void set_error(inner_tag, Rec &rec, R &&...result)
            {
                cleanup();
                rec.set_error(std::forward<R>(result)...);
            }

            F mF;
            ManualLifetime<Tuple> mValue;
            ManualLifetime<inner_state> mInnerState;
        };

        template <Sender Sender, typename F>
        struct sender : algorithm_sender<Sender> {
            template <typename... V>
            using helper = std::invoke_result_t<F, V...>;
            using inner_sender_t = typename Sender::template value_types<helper>;

            template <template <typename...> typename Tuple>
            using value_types = typename std::remove_reference_t<inner_sender_t>::template value_types<Tuple>;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Rec, Sender, F> { std::forward<Rec>(rec), std::forward<Sender>(sender.mSender), std::forward<F>(sender.mF) };
            }

            F mF;
        };

        template <Sender Sender, typename F>
        friend auto tag_invoke(let_value_t, Sender &&inner, F &&f)
        {
            return sender<Sender, F> { { {}, std::forward<Sender>(inner) }, std::forward<F>(f) };
        }

        template <typename Sender, typename F>
            requires tag_invocable<let_value_t, Sender, F>
        auto operator()(Sender &&sender, F &&f) const
            noexcept(is_nothrow_tag_invocable_v<let_value_t, Sender, F>)
                -> tag_invoke_result_t<let_value_t, Sender, F>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), std::forward<F>(f));
        }

        template <typename F>
        auto operator()(F &&f) const
        {
            return pipable_from_right(*this, std::forward<F>(f));
        }

        template <typename F>
        struct typed {
            template <typename Sender>
                requires tag_invocable<let_value_t, Sender, F>
            auto operator()(Sender &&sender, F &&finally = {}) const
                noexcept(is_nothrow_tag_invocable_v<let_value_t, Sender, F>)
                    -> tag_invoke_result_t<let_value_t, Sender, F>
            {
                return tag_invoke(let_value_t {}, std::forward<Sender>(sender), std::forward<F>(finally));
            }

            auto operator()(F &&finally = {}) const
            {
                return pipable_from_right(let_value_t {}, std::forward<F>(finally));
            }
        };

        template <typename Sender, typename F>
        struct Inner {
            friend auto tag_invoke(const Inner<Sender, F> &)
            {
                return sender<Sender, F> {};
            }

            auto operator()() const
                noexcept(is_nothrow_tag_invocable_v<Inner<Sender, F>>)
                    -> tag_invoke_result_t<Inner<Sender, F>>
            {
                return tag_invoke(*this);
            }
        };
    };

    inline constexpr let_value_t let_value;

    struct assign_t {

        template <typename Rec, typename Sender, typename T>
        struct state;

        template <typename Rec, typename Sender, typename T>
        struct receiver : algorithm_receiver<Rec> {

            template <typename... V>
            void set_value(V &&...value)
            {
                mState->set_value(this->mRec, std::forward<V>(value)...);
            }

            void set_done()
            {
                mState->set_done(this->mRec);
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mState->set_error(this->mRec, std::forward<R>(result)...);
            }

            state<Rec, Sender, T> *mState;
        };

        template <typename Rec, typename Sender, typename T>
        struct state : algorithm_state<Sender, receiver<Rec, Sender, T>> {

            state(Rec &&rec, Sender &&sender, T &&var)
                : algorithm_state<Sender, receiver<Rec, Sender, T>>(std::forward<Sender>(sender), std::forward<Rec>(rec), this)
                , mVar(std::forward<T>(var))
            {
            }

            template <typename V>
            void set_value(Rec &rec, V &&value)
            {
                mVar = std::forward<V>(value);
                rec.set_value();
            }

            T mVar;
        };

        template <typename Sender, typename T>
        struct sender : algorithm_sender<Sender> {

            template <template <typename...> typename Tuple>
            using value_types = Tuple<>;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Rec, Sender, T> { std::forward<Rec>(rec), std::forward<Sender>(sender.mSender), std::forward<T>(sender.mVar) };
            }

            T mVar;
        };

        template <typename Sender, typename T>
        friend auto tag_invoke(assign_t, Sender &&inner, T &&var)
        {
            return sender<Sender, T> { { {}, std::forward<Sender>(inner) }, std::forward<T>(var) };
        }

        template <typename Sender, typename T>
        auto operator()(Sender &&sender, T &&var) const
            noexcept(is_nothrow_tag_invocable_v<assign_t, Sender, T>)
                -> tag_invoke_result_t<assign_t, Sender, T>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), std::forward<T>(var));
        }

        template <typename T>
        auto operator()(T &var) const
        {
            return pipable_from_right(*this, var);
        }
    };

    inline constexpr assign_t assign;

    template <typename R>
    struct to_result_t {

        template <typename Rec, typename Sender>
        struct state;

        template <typename Rec, typename Sender>
        struct receiver : algorithm_receiver<Rec> {

            template <typename... V>
            void set_value(V &&...value)
            {
                this->mRec.set_value(mOnValue, std::forward<V>(value)...);
            }

            void set_done()
            {
                this->mRec.set_value(mOnDone);
            }

            template <typename... R2>
            void set_error(R fixedResult, R2 &&...result)
            {
                this->mRec.set_value(std::forward<R>(fixedResult), std::forward<R2>(result)...);
            }

            R mOnValue;
            R mOnDone;
        };

        template <typename Rec, typename Sender>
        struct state : algorithm_state<Sender, receiver<Rec, Sender>> {

            state(Rec &&rec, Sender &&sender, R onValue, R onDone)
                : algorithm_state<Sender, receiver<Rec, Sender>>(std::forward<Sender>(sender), std::forward<Rec>(rec), onValue, onDone)
            {
            }
        };

        template <typename Sender>
        struct sender : algorithm_sender<Sender> {

            template <template <typename...> typename Tuple>
            using value_types = typename Sender::template value_types<type_pack>::template prepend<R>::template instantiate<Tuple>;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Rec, Sender> { std::forward<Rec>(rec), std::forward<Sender>(sender.mSender), sender.mOnValue, sender.mOnDone };
            }

            R mOnValue;
            R mOnDone;
        };

        template <typename Sender>
        friend auto tag_invoke(to_result_t, Sender &&inner, R onValue, R onDone)
        {
            return sender<Sender> { { {}, std::forward<Sender>(inner) }, onValue, onDone };
        }

        template <typename Sender>
        auto operator()(Sender &&sender, R onValue, R onDone) const
            noexcept(is_nothrow_tag_invocable_v<to_result_t, Sender, R, R>)
                -> tag_invoke_result_t<to_result_t, Sender, R, R>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), onValue, onDone);
        }

        auto operator()(R onValue, R onDone) const
        {
            return pipable_from_right(*this, onValue, onDone);
        }
    };

    template <typename R>
    inline constexpr to_result_t<R> to_result;

    struct with_query_value_t {

        template <typename Rec, typename CPO, typename T>
        struct receiver : algorithm_receiver<Rec> {

            receiver(Rec &&rec, T &&queryResult)
                : algorithm_receiver<Rec>(std::forward<Rec>(rec))
                , mQueryResult(std::forward<T>(queryResult))
            {
            }

            friend T tag_invoke(CPO, receiver &rec)
            {
                return rec.mQueryResult;
            }

            T mQueryResult;
        };

        template <typename Sender, typename CPO, typename T>
        struct sender : algorithm_sender<Sender> {

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return algorithm_state<Sender, receiver<Rec, CPO, T>> { std::forward<Sender>(sender.mSender), std::forward<Rec>(rec), std::forward<T>(sender.mQueryResult) };
            }

            T mQueryResult;
        };

        template <typename Sender, typename CPO, typename T>
        friend auto tag_invoke(with_query_value_t, Sender &&inner, CPO cpo, T &&queryResult)
        {
            return sender<Sender, CPO, T> { { {}, std::forward<Sender>(inner) }, std::forward<T>(queryResult) };
        }

        template <typename Sender, typename CPO, typename T>
            requires tag_invocable<with_query_value_t, Sender, CPO, T>
        auto operator()(Sender &&sender, CPO cpo, T &&queryResult) const
            noexcept(is_nothrow_tag_invocable_v<with_query_value_t, Sender, CPO, T>)
                -> tag_invoke_result_t<with_query_value_t, Sender, CPO, T>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), cpo, std::forward<T>(queryResult));
        }

        template <typename CPO, typename T>
        auto operator()(CPO cpo, T &&queryResult) const
        {
            return pipable_from_right(*this, cpo, std::forward<T>(queryResult));
        }
    };

    inline constexpr with_query_value_t with_query_value;

    struct stop_when_t {

        template <typename Rec, typename Inner, typename Trigger>
        struct state;

        template <typename Rec, typename Inner, typename Trigger>
        struct receiver {

            template <typename... V>
            void set_value(V &&...value)
            {
                mState->set_value(std::forward<V>(value)...);
            }

            void set_done()
            {
                mState->set_done();
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mState->set_error(std::forward<R>(result)...);
            }

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, receiver &rec, Args &&...args)
                -> tag_invoke_result_t<CPO, Rec &, Args...>
            {
                return f(rec.mState->mRec, std::forward<Args>(args)...);
            }

            friend auto tag_invoke(get_stop_token_t, receiver &rec)
            {
                return rec.mState->mStopSource.get_token();
            }

            state<Rec, Inner, Trigger> *mState;
        };

        template <typename Rec, typename Inner, typename Trigger>
        struct stop_receiver {
            void set_value(auto &&...)
            {
                mState->stop();
            }

            void set_done()
            {
                mState->stop();
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mState->stop();
            }

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, stop_receiver &rec, Args &&...args)
                -> tag_invoke_result_t<CPO, Rec &, Args...>
            {
                return f(rec.mState->mRec, std::forward<Args>(args)...);
            }

            friend auto tag_invoke(get_stop_token_t, stop_receiver &rec)
            {
                return rec.mState->mStopSource.get_token();
            }

            state<Rec, Inner, Trigger> *mState;
        };

        template <typename Rec, typename Inner, typename Trigger>
        struct state {

            using inner_state = connect_result_t<Inner, receiver<Rec, Inner, Trigger>>;
            using stop_state = connect_result_t<Trigger, stop_receiver<Rec, Inner, Trigger>>;

            state(Rec &&rec, Inner &&sender, Trigger &&trigger)
                : mRec(std::forward<Rec>(rec))
                , mInnerState(connect(std::forward<Inner>(sender), receiver<Rec, Inner, Trigger> { this }))
                , mStopState(connect(std::forward<Trigger>(trigger), stop_receiver<Rec, Inner, Trigger> { this }))
            {
            }

            ~state() { }

            void start()
            {
                mInnerState.start();
                mStopState.start();
            }

            template <typename... V>
            void set_value(V &&...values)
            {
                if (mStopSource.request_stop()) {
                    mResult.set_value(std::forward<V>(values)...);
                }
                signal();
            }
            void set_done()
            {
                if (mStopSource.request_stop()) {
                    mResult.set_done();
                }
                signal();
            }
            template <typename... R>
            void set_error(R &&...results)
            {
                if (mStopSource.request_stop()) {
                    mResult.set_error(std::forward<R>(results)...);
                }
                signal();
            }

            void stop()
            {
                mStopSource.request_stop();
                signal();
            }

            void signal()
            {
                if (mFinished.test_and_set()) {
                    if (mResult.is_null()) {
                        mRec.set_value();
                    } else {
                        mResult.reproduce(mRec);
                    }
                }
            }

            ResultStorage<Inner> mResult;
            Rec mRec;
            std::stop_source mStopSource;
            inner_state mInnerState;
            stop_state mStopState;
            // stop_callback<> mPropagateCallback;
            std::atomic_flag mFinished;
        };

        template <Sender Inner, Sender Trigger>
        struct sender : algorithm_sender<Inner> {

            template <template <typename...> typename Tuple>
            using value_types = typename Inner::template value_types<Tuple>;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Rec, Inner, Trigger> { std::forward<Rec>(rec), std::forward<Inner>(sender.mSender), std::forward<Trigger>(sender.mTrigger) };
            }

            Trigger mTrigger;
        };

        template <Sender Inner, Sender Trigger>
        friend auto tag_invoke(stop_when_t, Inner &&inner, Trigger &&trigger)
        {
            return sender<Inner, Trigger> { { {}, std::forward<Inner>(inner) }, std::forward<Trigger>(trigger) };
        }

        template <Sender Inner, Sender Trigger>
            requires tag_invocable<stop_when_t, Inner, Trigger>
        auto operator()(Inner &&sender, Trigger &&trigger) const
            noexcept(is_nothrow_tag_invocable_v<stop_when_t, Inner, Trigger>)
                -> tag_invoke_result_t<stop_when_t, Inner, Trigger>
        {
            return tag_invoke(*this, std::forward<Inner>(sender), std::forward<Trigger>(trigger));
        }

        template <Sender Trigger>
        auto operator()(Trigger &&trigger) const
        {
            return pipable_from_right(*this, std::forward<Trigger>(trigger));
        }
    };

    inline constexpr stop_when_t stop_when;

}
}