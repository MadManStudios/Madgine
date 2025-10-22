#include "behaviorlib.h"

#include "parametertuple.h"

#include "Meta/keyvalue/metatable_impl.h"

METATABLE_BEGIN(Engine::ParameterTuple)
METATABLE_END(Engine::ParameterTuple)

namespace Engine {

ScopePtr ParameterTuple::customScopePtr()
{
    return mTuple->customScopePtr();
}


namespace Serialize {

    StreamResult Operations<ParameterTuple>::read(Serialize::CallerHierarchyFormattedSerializeStream in, ParameterTuple &tuple, const char *name)
    {
        STREAM_PROPAGATE_ERROR(in.mStream.beginCompoundRead(name));
        STREAM_PROPAGATE_ERROR(tuple.mTuple->read(in));
        return in.mStream.endCompoundRead(name);
    }

    void Operations<ParameterTuple>::write(Serialize::CallerHierarchyFormattedSerializeStream out, const ParameterTuple &tuple, const char *name)
    {
        out.mStream.beginCompoundWrite(name);
        tuple.mTuple->write(out);
        out.mStream.endCompoundWrite(name);
    }

    StreamResult Operations<ParameterTuple>::visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        throw 0;
        return {};
    }

}

}