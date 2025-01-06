#include "../debuglib.h"

#include "debuggablelifetime.h"

#include "Modules/threading/workgroupstorage.h"

#include "Madgine/root/keyvalueregistry.h"

#include "Meta/keyvalue/metatable_impl.h"

METATABLE_BEGIN(Engine::Debug::DebuggableLifetimeBase)
READONLY_PROPERTY(Owner, owner)
READONLY_PROPERTY(Children, children)
METATABLE_END(Engine::Debug::DebuggableLifetimeBase)

static_assert(std::input_or_output_iterator<Engine::Generator<Engine::Debug::DebuggableLifetimeBase &>::iterator>);
static_assert(std::ranges::range<Engine::Generator<Engine::Debug::DebuggableLifetimeBase &>>);

namespace Engine {
namespace Debug {

    struct RootLifetime : DebuggableLifetimeBase {

        RootLifetime()
            : DebuggableLifetimeBase(std::nullopt)
        {
        }

        void startLifetime() override
        {
            for (DebuggableLifetimeBase &child : children()) {
                child.startLifetime();
            }
        }
        void endLifetime() override
        {
            for (DebuggableLifetimeBase &child : children()) {
                child.endLifetime();
            }
        }
        ScopePtr owner() override
        {
            return {};
        }
    };

    Threading::WorkgroupLocal<RootLifetime> sRoot;
    Threading::WorkgroupLocal<bool> sInitialized;

    DebuggableLifetimeBase *getRoot()
    {
        if (!sInitialized) {
            sInitialized = true;
            KeyValueRegistry::registerWorkGroupLocal("Lifetimes", &sRoot);
        }
        return &sRoot;
    }

    DebuggableLifetimeBase::DebuggableLifetimeBase(DebuggableLifetimeBase *parent)
        : mParent(parent ? parent : getRoot())
        , mPrev(mParent->mLastChild)
    {
        if (mParent->mLastChild) {
            mParent->mLastChild->mNext = this;
        } else {
            assert(!mParent->mFirstChild);
            mParent->mFirstChild = this;
        }
        mParent->mLastChild = this;
    }

    DebuggableLifetimeBase::DebuggableLifetimeBase(std::nullopt_t)
    {
    }

    Generator<DebuggableLifetimeBase &> DebuggableLifetimeBase::children()
    {
        DebuggableLifetimeBase *child = mFirstChild;
        while (child) {
            co_yield *child;
            child = child->mNext;
        }
    }

    const std::vector<std::reference_wrapper<ContextInfo>> &DebuggableLifetimeBase::debugContexts()
    {
        return mDebugContexts;
    }

}
}

METATABLE_BEGIN_BASE(Engine::Debug::RootLifetime, Engine::Debug::DebuggableLifetimeBase)
METATABLE_END(Engine::Debug::RootLifetime)
