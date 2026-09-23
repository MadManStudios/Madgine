#pragma once

#include "Generic/typed_ptr.h"

#include "contextinfo.h"
#include "debuglocation.h"

namespace Engine {
namespace Debug {

    enum class ContinuationMode {
        Continue,
        Step,
        StepInto,
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

            virtual void call() = 0;

            virtual void visitArguments(std::ostream &) = 0;
        };

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

            virtual void call() override
            {
                TupleUnpacker::invokeFromTuple(std::forward<F>(mCallback), std::move(mArgs));
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
        Continuation(const Continuation &) = delete;
        ~Continuation()
        {
            assert(!implPtr());
        }

        template <typename Rec, typename F, typename... Args>
        void suspend(TypedPtr location, Rec &rec, F &&callback, ContinuationType type, Args &&...args)
        {
            auto f = [&rec, this, callback { forward_capture<F>(callback) }](Args &&...args) mutable {
                if (mode() == Debug::ContinuationMode::Abort) {
                    rec.set_done();
                } else {
                    std::forward<F>(callback)(rec, std::forward<Args>(args)...);
                }
            };

            Base *impl = new Impl<decltype(f), Args...>(std::move(f), std::forward<Args>(args)...);

            uintptr_t prev = mImpl.exchange(reinterpret_cast<uintptr_t>(impl) | static_cast<uintptr_t>(type));
            assert(!toImpl(prev));

            get_debug_context(rec).suspend(location, type);
        }

        template <typename Rec, typename F, typename... Args>
        void pass(TypedPtr location, Rec &rec, F &&callback, ContinuationType type, IndexType<size_t> line = {}, Args &&...args)
        {
            if (mode() == ContinuationMode::Abort) {
                rec.set_done();
            } else {
                if (mode() != ContinuationMode::Continue || get_debug_context(rec).wantsPause(location, type, line)) {
                    suspend(location, rec, std::forward<F>(callback), type, std::forward<Args>(args)...);
                } else {
                    std::forward<F>(callback)(rec, std::forward<Args>(args)...);
                }
            }
        }

        explicit operator bool() const
        {
            Base *impl = implPtr();
            return impl;
        }

        bool stop()
        {
            uintptr_t impl = mImpl.exchange(static_cast<uintptr_t>(ContinuationMode::Abort));
            Base *ptr = toImpl(impl);
            if (ptr) {
                ptr->call();
                delete ptr;
                return true;
            }
            return false;
        }

        ContinuationType type() const
        {
            uintptr_t impl = mImpl.load();
            assert(toImpl(impl));
            return static_cast<ContinuationType>(impl & 0x7);
        }

        void setMode(ContinuationMode mode)
        {
            uintptr_t impl = mImpl.exchange(static_cast<uintptr_t>(mode));
            assert(!toImpl(impl));
        }

        ContinuationMode mode() const
        {
            uintptr_t impl = mImpl.load();
            assert(!toImpl(impl));
            return static_cast<ContinuationMode>(impl & 0x7);
        }

        void operator()(ContinuationMode mode)
        {
            uintptr_t prev = mImpl.exchange(static_cast<uintptr_t>(mode));

            Base *impl = toImpl(prev);
            assert(impl);
            impl->call();
            delete impl;
        }

        void visitArguments(std::ostream &out) const
        {
            Base *impl = implPtr();
            assert(impl);
            impl->visitArguments(out);
        }

    protected:
        static Base *toImpl(uintptr_t p)
        {
            return reinterpret_cast<Base *>(p & ~0x7);
        }

        Base *implPtr() const
        {
            return toImpl(mImpl.load());
        }

    private:
        std::atomic<uintptr_t> mImpl = 0;
    };

}
}