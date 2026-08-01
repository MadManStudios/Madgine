#include "../debuglib.h"

#include "debuggablelifetime.h"

#include "Modules/threading/workgroupstorage.h"

#include "Madgine/root/keyvalueregistry.h"

#include "Meta/reflect/metatable_impl.h"

#include "debugger.h"

METATABLE_BEGIN(Engine::Debug::DebuggableLifetimeBase)
    //READONLY_PROPERTY(Owner, owner)
    READONLY_PROPERTY(Children, children)
    READONLY_PROPERTY(DebugContexts, debugContexts)
METATABLE_END(Engine::Debug::DebuggableLifetimeBase)

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

        Reflect::ScopePtr owner() override
        {
            return {};
        }

        bool running() override
        {
            for (DebuggableLifetimeBase &child : children()) {
                if (child.running())
                    return true;
            }
            return false;
        }
    };

    Threading::WorkgroupLocal<RootLifetime> sRoot;
    Threading::WorkgroupLocal<bool> sInitialized;

    MADGINE_DEBUGGER_EXPORT DebuggableLifetimeBase &getRootLifetime()
    {
        if (!sInitialized) {
            sInitialized = true;
            Core::KeyValueRegistry::registerWorkGroupLocal("Lifetimes", &sRoot);
        }
        return sRoot;
    }

    DebuggableLifetimeBase::DebuggableLifetimeBase(DebuggableLifetimeBase *parent)
        : mParent(parent ? parent : &getRootLifetime())
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

    DebuggableLifetimeBase::~DebuggableLifetimeBase()
    {
        if (mParent) {
            if (mParent->mLastChild == this)
                mParent->mLastChild = mPrev;
            if (mParent->mFirstChild == this)
                mParent->mFirstChild = mNext;

            if (mPrev)
                mPrev->mNext = mNext;
            if (mNext)
                mNext->mPrev = mPrev;
        }
    }

    DebuggableLifetimeBase::DebuggableLifetimeBase(std::nullopt_t)
    {
    }

    DebuggableLifetimeBase *DebuggableLifetimeBase::parent() const
    {
        return mParent;
    }

    std::ranges::subrange<DebuggableLifetimeBase::iterator, DebuggableLifetimeBase::iterator> DebuggableLifetimeBase::children()
    {
        return { mFirstChild, nullptr };
    }

    const std::vector<std::reference_wrapper<ContextInfo>> &DebuggableLifetimeBase::debugContexts()
    {
        return mDebugContexts;
    }

    ContextInfo &DebuggableLifetimeBase::createContext()
    {
        ContextInfo &context = Debugger::getSingleton().createContext();
        mDebugContexts.emplace_back(context);
        return context;
    }

    DebuggableLifetimeBase::iterator::iterator() = default;
    DebuggableLifetimeBase::iterator::iterator(DebuggableLifetimeBase *current)
        : mCurrent(current)
    {
    }

    DebuggableLifetimeBase &DebuggableLifetimeBase::iterator::operator*() const
    {
        return *mCurrent;
    }

    DebuggableLifetimeBase::iterator &DebuggableLifetimeBase::iterator::operator++()
    {
        mCurrent = mCurrent->mNext;
        return *this;
    }
    DebuggableLifetimeBase::iterator DebuggableLifetimeBase::iterator::operator++(int)
    {
        iterator copy = *this;
        mCurrent = mCurrent->mNext;
        return copy;
    }

    bool DebuggableLifetimeBase::iterator::operator==(const DebuggableLifetimeBase::iterator &other) const
    {
        return mCurrent == other.mCurrent;
    }

}
}

METATABLE_BEGIN_BASE(Engine::Debug::RootLifetime, Engine::Debug::DebuggableLifetimeBase)
METATABLE_END(Engine::Debug::RootLifetime)
