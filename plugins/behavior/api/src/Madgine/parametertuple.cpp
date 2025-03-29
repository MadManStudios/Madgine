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

    StreamResult Operations<ParameterTuple>::read(Serialize::FormattedSerializeStream &in, ParameterTuple &tuple, const char *name)
    {
        STREAM_PROPAGATE_ERROR(in.beginCompoundRead(name));
        STREAM_PROPAGATE_ERROR(tuple.mTuple->read(in));
        return in.endCompoundRead(name);
    }

    void Operations<ParameterTuple>::write(Serialize::FormattedSerializeStream &out, const ParameterTuple &tuple, const char *name)
    {
        out.beginCompoundWrite(name);
        tuple.mTuple->write(out);
        out.endCompoundWrite(name);
    }

    StreamResult Operations<ParameterTuple>::visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
    {
        throw 0;
        return {};
    }

}

}