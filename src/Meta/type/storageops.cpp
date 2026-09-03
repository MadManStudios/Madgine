#include "../metalib.h"

#include "storageops.h"

namespace Engine {
namespace Type {

    Reflect::Result StorageOps::construct(BaseStorage &storage, const Reflect::ArgumentList &args, size_t inlineSize) const
    {
        IndexType<size_t> matchedIndex;

        for (size_t i = 0; mConstructors[i].mMatcher; ++i) {
            if (mConstructors[i].mMatcher(*this, args)) {
                if (matchedIndex) {
                    return REFLECT_UNKNOWN_ERROR() << "Ambiguous constructors for " << mTypeName;
                }
                matchedIndex = i;
            }
        }

        if (!matchedIndex) {
            return REFLECT_UNKNOWN_ERROR() << "No matching constructor for " << mTypeName;
        }

        return mConstructors[matchedIndex].mInvoker(*this, storage, args, inlineSize);
    }

    Serialize::StreamResult StorageOps::read(Serialize::FormattedSerializeStream &in, BaseStorage &storage, const char *name, size_t inlineSize, Serialize::ContextPtr context) const
    {
        return mRead(*this, in, storage, name, inlineSize, context);
    }

}

namespace Serialize {

    void Operations<const Type::StorageOps *>::write(FormattedSerializeStream &out, const Type::StorageOps *const &t, const char *name, ContextPtr context)
    {
        Serialize::write(out, t->mTypeName, name, context);
    }

    StreamResult Operations<const Type::StorageOps *>::read(FormattedSerializeStream &in, const Type::StorageOps *&t, const char *name, ContextPtr context)
    {
        std::string typeName;
        STREAM_PROPAGATE_ERROR(Serialize::read(in, typeName, name, context));
        const Type::TypeName *type = Type::resolveTypeName(typeName);
        if (!type)
            throw 0;
        if (!type->mMetaTable)
            throw 0;
        if (!type->mMetaTable->mStorage)
            throw 0;
        t = *type->mMetaTable->mStorage;
        return {};
    }

    StreamResult Operations<const Type::StorageOps *>::visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        throw 0;
        return {};
    }

}
}