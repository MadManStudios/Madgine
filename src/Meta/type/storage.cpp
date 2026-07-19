#include "../metalib.h"

#include "../serialize/streams/streamresult.h"
#include "storageops.h"

namespace Engine {
namespace Type {

    void StorageDeleter::operator()(BaseStorage *alloc) const
    {
        alloc->mType.mDestructor(alloc, std::numeric_limits<size_t>::max());
    }

    InlineStorage::InlineStorage(const StorageOps &ops, const Reflect::ArgumentList &args)
        : BaseStorage(ops)
    {
        Reflect::Result result = ops.construct(*this, args, sizeof(mDummy));
        assert(!result);
    }

    InlineStorage::~InlineStorage()
    {
        mType.mDestructor(this, sizeof(mDummy));
        assert(mDummy[0] == 0 && mDummy[1] == 0);
    }

    AllocationStorage::AllocationStorage(const StorageOps &ops, const Reflect::ArgumentList &args)
        : BaseStorage(ops)
    {
        Reflect::Result result = ops.mConstructors[0].mInvoker(ops, reinterpret_cast<InlineStorage &>(*this), args, 0);
        assert(!result);
    }

    void BaseStorage::toValue(Reflect::Value &retVal)
    {
        mType.mToValue(retVal, *this, std::numeric_limits<size_t>::max());
    }

    Reflect::Result BaseStorage::fromValue(const Reflect::Value &value)
    {
        return mType.mFromValue(*this, value, std::numeric_limits<size_t>::max());
    }

    void InlineStorage::toValue(Reflect::Value &retVal)
    {
        return mType.mToValue(retVal, *this, sizeof(mDummy));
    }

    Reflect::Result InlineStorage::fromValue(const Reflect::Value &value)
    {
        return mType.mFromValue(*this, value, sizeof(mDummy));
    }

    void AllocationStorage::toValue(Reflect::Value &retVal)
    {
        mType.mToValue(retVal, *this, 0);
    }

    Reflect::Result AllocationStorage::fromValue(const Reflect::Value &value)
    {
        return mType.mFromValue(*this, value, 0);
    }

    Serialize::StreamResult AllocationStorage::read(Serialize::CallerHierarchyFormattedSerializeStream in, const char *name)
    {
        return mAllocation->mType.mRead(mAllocation->mType, in, *mAllocation, name, 0);
    }

    void AllocationStorage::write(Serialize::CallerHierarchyFormattedSerializeStream out, const char *name) const
    {
        mAllocation->mType.mWrite(out, *mAllocation, name, 0);
    }

    Serialize::StreamResult AllocationStorage::applyMap(Serialize::CallerHierarchyFormattedSerializeStream in, bool success)
    {
        return mAllocation->mType.mApplyMap(*mAllocation, in, success, 0);
    }

    Serialize::StreamResult tag_invoke(const Serialize::apply_map_t &, InlineStorage &storage, Serialize::CallerHierarchyFormattedSerializeStream in, bool success)
    {
        return storage.mType.mApplyMap(storage, in, success, sizeof(storage.mDummy));
    }
}

namespace Serialize {
    StreamResult Operations<Type::InlineStorage>::read(Serialize::CallerHierarchyFormattedSerializeStream in, Type::InlineStorage &storage, const char *name)
    {
        return storage.mType.read(in, storage, name, sizeof(storage.mDummy));
    }

    void Operations<Type::InlineStorage>::write(Serialize::CallerHierarchyFormattedSerializeStream out, const Type::InlineStorage &storage, const char *name)
    {
        storage.mType.mWrite(out, storage, name, sizeof(storage.mDummy));
    }

    StreamResult Operations<Type::InlineStorage>::visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        throw "TODO";
    }
}
}