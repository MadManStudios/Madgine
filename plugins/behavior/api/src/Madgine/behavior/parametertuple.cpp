#include "../behaviorlib.h"

#include "parametertuple.h"

#include "Meta/serialize/streams/formattedserializestream.h"

#include "Meta/reflect/metatable_impl.h"

#include "dynamicparametertuple.h"

METATABLE_BEGIN(Engine::Behavior::ParameterTuple)
METATABLE_END(Engine::Behavior::ParameterTuple)

namespace Engine {
namespace Behavior {

    ParameterTuple::ParameterTuple(const Reflect::MetaTable &metaTable, const std::vector<BehaviorDescriptor::Parameter> &parameters)
        : mTuple(std::make_unique<DynamicParameterTuple>(metaTable, parameters))
    {
    }

    Reflect::ScopePtr ParameterTuple::customScopePtr()
    {
        return mTuple->customScopePtr();
    }

    Serialize::StreamResult tag_invoke(const Serialize::apply_map_t &, ParameterTuple &tuple, Serialize::FormattedSerializeStream &in, bool success, Serialize::ContextPtr context)
    {
        return tuple.mTuple->applyMap(in, success, context);
    }

}

namespace Serialize {

    StreamResult Operations<Behavior::ParameterTuple>::read(Serialize::FormattedSerializeStream &in, Behavior::ParameterTuple &tuple, const char *name, ContextPtr context)
    {
        STREAM_PROPAGATE_ERROR(in.beginCompoundRead(name));
        STREAM_PROPAGATE_ERROR(tuple.mTuple->read(in, context));
        return in.endCompoundRead(name);
    }

    void Operations<Behavior::ParameterTuple>::write(Serialize::FormattedSerializeStream &out, const Behavior::ParameterTuple &tuple, const char *name, ContextPtr context)
    {
        out.beginCompoundWrite(name);
        tuple.mTuple->write(out, context);
        out.endCompoundWrite(name);
    }

    StreamResult Operations<Behavior::ParameterTuple>::visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        throw 0;
        return {};
    }

}

}