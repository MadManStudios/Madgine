#pragma once

namespace Engine {
namespace Debug {

    enum class ContinuationMode {
        Continue,
        Abort
    };

    enum class ContinuationType {
        Flow,
        Return,
        Error,
        Cancelled
    };

    struct [[nodiscard]] Continuation {
    private:
        struct Base {
            virtual ~Base() = default;

            virtual void call(ContinuationMode mode) = 0;

            virtual void visitArguments(std::ostream &) = 0;
        };

        static inline Base *const sAborted = reinterpret_cast<Base *>(0x1);

        template <typename F, typename... Args>
        struct Impl : Base {
            Impl(F &&callback, Args &&...args)
                : mCallback(std::forward<F>(callback))
                , mArgs { std::forward<Args>(args)... }
            {
            }

            /* virtual void call(ContinuationMode mode) override
            {
                switch (mode) {
                case Debug::ContinuationMode::Continue:
                    TupleUnpacker::invokeExpand(std::forward<F>(mCallback), mRec, std::move(mArgs));
                    break;
                case Debug::ContinuationMode::Abort:
                    mRec.set_done();
                    break;
                default:
                    throw 0;
                }
            }*/

            virtual void call(ContinuationMode mode) override
            {
                TupleUnpacker::invokeExpand(std::forward<F>(mCallback), mode, std::move(mArgs));
            }

            virtual void visitArguments(std::ostream &out) override
            {
                StringUtil::StreamJoiner joiner { out, "\n" };
                TupleUnpacker::forEach(mArgs, [&](auto &v) {
                    if constexpr (requires { out << v; }) {
                        joiner.next() << v;
                    } else {
                        joiner.next() << typeid(v).name();
                    }
                });
            }

            F mCallback;
            std::tuple<Args...> mArgs;
        };

    public:
        Continuation() = default;

        template <typename F, typename... Args>
        Continuation(F &&callback, ContinuationType type, Args &&...args)
            : mImpl(new Impl<F, Args...>(std::forward<F>(callback), std::forward<Args>(args)...))
            , mType(type)
        {
        }
        Continuation(const Continuation &) = delete;
        Continuation(Continuation &&other)
        {
            Base *otherImpl = other.mImpl.exchange(nullptr);
            assert(otherImpl != sAborted);
            mImpl = otherImpl;
        }
        ~Continuation()
        {
            assert(!*this);
        }

        template <typename Rec, typename F, typename... Args>
        static Continuation fromPromise(Rec &rec, F &&callback, ContinuationType type, Args &&...args)
        {
            return {
                [&rec, callback { forward_capture<F>(callback) }](ContinuationMode mode, Args &&...args) mutable {
                    switch (mode) {
                    case Debug::ContinuationMode::Continue:
                        std::forward<F>(callback)(rec, std::forward<Args>(args)...);
                        break;
                    case Debug::ContinuationMode::Abort:
                        rec.set_done();
                        break;
                    default:
                        throw 0;
                    }
                },
                type, std::forward<Args>(args)...
            };
        }

        Continuation &operator=(Continuation &&other)
        {
            Base *otherImpl = other.mImpl.exchange(nullptr);
            assert(otherImpl != sAborted);
            if (otherImpl) {
                Base *expected = nullptr;
                if (!mImpl.compare_exchange_strong(expected, otherImpl)) {
                    assert(expected == sAborted);
                    otherImpl->call(ContinuationMode::Abort);
                    delete otherImpl;
                }
            }
            return *this;
        }

        explicit operator bool() const
        {
            Base *impl = mImpl.load();
            return impl && impl != sAborted;
        }

        void stop()
        {
            Base *impl = mImpl.exchange(sAborted);
            if (impl && impl != sAborted) {
                impl->call(ContinuationMode::Abort);
                delete impl;
            }
        }

        ContinuationType type() const
        {
            return mType;
        }

        void operator()(ContinuationMode mode)
        {
            Base *impl = mImpl.exchange(nullptr);
            assert(impl && impl != sAborted);
            impl->call(mode);
            delete impl;
        }

        void visitArguments(std::ostream &out) const
        {
            Base *impl = mImpl.load();
            assert(impl && impl != sAborted);
            impl->visitArguments(out);
        }

    private:
        std::atomic<Base *> mImpl = nullptr;
        ContinuationType mType;
    };

}
}