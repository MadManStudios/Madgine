#include "../behaviorlib.h"

#include "parametertuple.h"

#include "Meta/keyvalue/metatable_impl.h"

METATABLE_BEGIN(Engine::Behavior::ParameterTuple)
METATABLE_END(Engine::Behavior::ParameterTuple)

namespace Engine {
namespace Behavior {

    ParameterTuple::ParameterTuple(std::unique_ptr<ParameterTupleBase> tuple)
        : mTuple(std::move(tuple))
    {
    }

    ScopePtr ParameterTuple::customScopePtr()
    {
        return mTuple->customScopePtr();
    }

}

namespace Serialize {

    StreamResult Operations<Behavior::ParameterTuple>::read(Serialize::CallerHierarchyFormattedSerializeStream in, Behavior::ParameterTuple &tuple, const char *name)
    {
        STREAM_PROPAGATE_ERROR(in.mStream.beginCompoundRead(name));
        STREAM_PROPAGATE_ERROR(tuple.mTuple->read(in));
        return in.mStream.endCompoundRead(name);
    }

    void Operations<Behavior::ParameterTuple>::write(Serialize::CallerHierarchyFormattedSerializeStream out, const Behavior::ParameterTuple &tuple, const char *name)
    {
        out.mStream.beginCompoundWrite(name);
        tuple.mTuple->write(out);
        out.mStream.endCompoundWrite(name);
    }

    StreamResult Operations<Behavior::ParameterTuple>::visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        throw 0;
        return {};
    }

}

}