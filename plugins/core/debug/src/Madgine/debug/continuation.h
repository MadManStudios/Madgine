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

    struct Continuation {
    private:
        struct Base {
            virtual ~Base() = default;

            virtual void call(ContinuationMode mode) = 0;

            virtual void visitArguments(std::ostream &) = 0;
        };

        template <typename F, typename... Args>
        struct Impl : Base {
            Impl(F &&callback, Args &&...args)
                : mCallback(std::forward<F>(callback))
                , mArgs { std::forward<Args>(args)... }
            {
            }

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
            : mImpl(std::make_unique<Impl<F, Args...>>(std::forward<F>(callback), std::forward<Args>(args)...))
            , mType(type)
        {
        }

        explicit operator bool() const
        {
            return static_cast<bool>(mImpl);
        }

        ContinuationType type() const
        {
            return mType;
        }

        void operator()(ContinuationMode mode)
        {
            mImpl->call(mode);
            mImpl.reset();
        }

        void visitArguments(std::ostream &out) const
        {
            mImpl->visitArguments(out);
        }

    private:
        std::unique_ptr<Base> mImpl;
        ContinuationType mType;
    };


}
}