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
}