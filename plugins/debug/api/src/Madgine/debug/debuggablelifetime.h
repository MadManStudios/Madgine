#pragma once

#include "Generic/execution/lifetime.h"
#include "Generic/memberoffsetptr.h"

#include "Meta/keyvalue/scopeptr.h"

#include "debuggablesender.h"

namespace Engine {
namespace Debug {

    struct MADGINE_DEBUGGER_EXPORT DebuggableLifetimeBase {
        DebuggableLifetimeBase(DebuggableLifetimeBase *parent = nullptr);
        ~DebuggableLifetimeBase();

        virtual void startLifetime() = 0;
        virtual void endLifetime() = 0;
        virtual bool running() = 0;

        Generator<DebuggableLifetimeBase &> children();

        virtual ScopePtr owner() = 0;

        const std::vector<std::reference_wrapper<ContextInfo>> &debugContexts();

    protected:
        DebuggableLifetimeBase(std::nullopt_t);

        ContextInfo &createContext();

    private:
        DebuggableLifetimeBase *mParent = nullptr;
        DebuggableLifetimeBase *mPrev = nullptr;
        DebuggableLifetimeBase *mNext = nullptr;
        DebuggableLifetimeBase *mFirstChild = nullptr;
        DebuggableLifetimeBase *mLastChild = nullptr;

        std::vector<std::reference_wrapper<ContextInfo>> mDebugContexts;
    };

    template <auto... cpos>
    struct DebuggableLifetime : DebuggableLifetimeBase {

        using DebuggableLifetimeBase::DebuggableLifetimeBase;

        template <Execution::AnySender Sender>
        void attach(Sender &&sender)
        {
            ContextInfo &context = createContext();
            mLifetime.attach(std::forward<Sender>(sender) | Execution::with_debug_location(context.mChild) | Debug::with_debug_context(context));
        }

        bool end()
        {
            return mLifetime.end();
        }

        bool running()
        {
            return mLifetime.running();
        }

        auto &finished()
        {
            return mLifetime.finished();
        }
        
        template <typename F>
        auto tracked(F &&callback)
        {
            return mLifetime.tracked(std::forward<F>(callback));
        }

        template <Execution::AnyBinding Binding>
        auto bind(Binding &&binding)
        {
            return mLifetime.bind(std::forward<Binding>(binding));
        }

        operator Execution::Lifetime<cpos...> &() &
        {
            return mLifetime;
        }

        using is_sender = void;

        using result_type = void;
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
