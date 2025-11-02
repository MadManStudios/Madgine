#include "../debuglib.h"

#include "debuglocation.h"

#include "debugger.h"


namespace Engine {
namespace Debug {

    
    DebugLocation *ParentLocation::currentLocation() const
    {
        if (!mChild)
            return nullptr;
        if (!mChild->mChild)
            return mChild;
        return mChild->currentLocation();
    }
	
    void DebugLocation::stepInto(ParentLocation *parent)
    {
        mContext = parent->mContext;
        std::unique_lock guard { mContext->mMutex };
        assert(!parent->mChild);
        parent->mChild = this;
    }

    void DebugLocation::stepOut(ParentLocation *parent)
    {
        std::unique_lock guard { mContext->mMutex };
        assert(parent->mChild == this);
        assert(!mChild);
        parent->mChild = nullptr;
    }

    bool DebugLocation::wantsPause(ContinuationType type, IndexType<size_t> line) const
    {
        return Debugger::getSingleton().wantsPause(*this, type, line);
    }

    void DebugLocation::yieldImpl(Continuation cont, Continuation &outContinuation, Execution::StopToken st)
    {
        mContext->suspend(std::move(cont), outContinuation, std::move(st));
    }


}
}