#pragma once

#include "Generic/memberoffsetptr.h"

#include "Generic/execution/lifetime.h"

namespace Engine {
namespace Debug {

    struct MADGINE_DEBUGGER_EXPORT DebuggableLifetimeBase {
        DebuggableLifetimeBase(DebuggableLifetimeBase *parent = nullptr);

        virtual void startLifetime() = 0;
        virtual void endLifetime() = 0;

        Generator<DebuggableLifetimeBase &> children();

        virtual ScopePtr owner() = 0;

    protected:
        DebuggableLifetimeBase(std::nullopt_t);

    private:
        DebuggableLifetimeBase *mParent = nullptr;
        DebuggableLifetimeBase *mPrev = nullptr;
        DebuggableLifetimeBase *mNext = nullptr;
        DebuggableLifetimeBase *mFirstChild = nullptr;
        DebuggableLifetimeBase *mLastChild = nullptr;
    };

    template <auto... cpos>
    struct DebuggableLifetime : DebuggableLifetimeBase {

        using DebuggableLifetimeBase::DebuggableLifetimeBase;

        template <Execution::Sender Sender>
        void attach(Sender &&sender)
        {
            mLifetime.attach(std::forward<Sender>(sender));
        }

        void start()
        {
            mLifetime.start();
        }

        auto end()
        {
            return mLifetime.end();
        }

        using is_sender = void;

        using result_type = GenericResult;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<>;

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t cpo, DebuggableLifetime &lifetime, Rec &&rec)
        {
            return tag_invoke(cpo, lifetime.mLifetime, std::forward<Rec>(rec));
        }

    private:
        typename Execution::Lifetime<cpos...> mLifetime;
    };

    template <typename OffsetPtr, typename CPOs>
    struct DebuggableLifetimeImpl : CPOs::template instantiate<DebuggableLifetime> {

        using CPOs::template instantiate<DebuggableLifetime>::instantiate;

        void startLifetime() override
        {
            OffsetPtr::parent(this)->startLifetime();
        }

        void endLifetime() override
        {
            OffsetPtr::parent(this)->endLifetime();
        }

        ScopePtr owner() override
        {
            return OffsetPtr::parent(this);
        }
    };

}
}

#define DEBUGGABLE_LIFETIME(Name, ...) MEMBER_OFFSET_CONTAINER(Name, , Engine::Debug::DebuggableLifetimeImpl<TaggedPlaceholder<MemberOffsetPtrTag, 0>, auto_pack<__VA_ARGS__>>);
