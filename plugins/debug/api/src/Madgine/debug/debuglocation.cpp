#include "../debuglib.h"

#include "debuglocation.h"

#include "debugger.h"

namespace Engine {
namespace Debug {

    void BaseLocation::stepInto(DebugLocation &child)
    {
        assert(!child.mContext);
        child.mContext = mContext;
    }

    void BaseLocation::stepOut(DebugLocation &child)
    {
        assert(child.mContext == mContext);
        child.mContext = nullptr;
    }

    void SimpleLocation::stepInto(DebugLocation &child)
    {
        std::unique_lock lock { mContext->mMutex };
        BaseLocation::stepInto(child);
        assert(!mChild);
        mChild = &child;
    }

    void SimpleLocation::stepOut(DebugLocation &child)
    {
        std::unique_lock lock { mContext->mMutex };
        assert(mChild == &child);
        mChild = nullptr;
        BaseLocation::stepOut(child);
    }

    bool DebugLocation::wantsPause(ContinuationType type, IndexType<size_t> line) const
    {
        return Debugger::getSingleton().wantsPause(*this, type, line);
    }

    void DebugLocation::yieldImpl(Continuation cont, Continuation &outContinuation, Execution::StopToken st)
    {
        mContext->suspend(*this, std::move(cont), outContinuation, std::move(st));
    }

}
}