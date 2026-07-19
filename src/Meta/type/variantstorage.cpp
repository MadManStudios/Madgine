#include "../metalib.h"

#include "../reflect/argumentlist.h"
#include "storageops.h"

namespace Engine {
namespace Type {

    struct VariantStorage : AllocationStorage {

        IndexType<uint32_t, 0> mIndex;
    };

    struct VariantStorageOps : StorageOps {

        static bool sMatcher(const StorageOps &, const Reflect::ArgumentList &)
        {
            return true;
        }

        static Reflect::Result sConstructor(const StorageOps &ops, BaseStorage &storage, const Reflect::ArgumentList &args, size_t inlineSize)
        {
            static_cast<VariantStorage &>(storage).mIndex.reset();
            assert(inlineSize > 0);
            assert(inlineSize >= sizeof(AllocationPtr) + sizeof(uintptr_t));
            Reflect::Result result = static_cast<const VariantStorageOps &>(ops).mTypeList[0]->construct(storage, args, sizeof(AllocationPtr));
            if (!result)
                static_cast<VariantStorage &>(storage).mIndex = 0 + 1;
            return result;
        }

        static void sMoveAssign(BaseStorage &, BaseStorage &&)
        {
            throw "TODO";
        }

        static void sToValue(Reflect::Value &retVal, BaseStorage &storage, size_t inlineSize)
        {
            assert(inlineSize > 0);
            assert(inlineSize >= sizeof(AllocationPtr) + sizeof(uintptr_t));

            static_cast<const VariantStorageOps &>(storage.mType).mTypeList[static_cast<VariantStorage &>(storage).mIndex - 1]->mToValue(retVal, storage, sizeof(AllocationPtr));
        }

        static Reflect::Result sFromValue(BaseStorage &, const Reflect::Value &val, size_t inlineSize)
        {
            throw "TODO";
        }

        static void sDestructor(BaseStorage *storage, size_t inlineSize)
        {
            assert(inlineSize > 0);
            assert(inlineSize >= sizeof(AllocationPtr) + sizeof(uintptr_t));

            static_cast<const VariantStorageOps &>(storage->mType).mTypeList[static_cast<VariantStorage *>(storage)->mIndex - 1]->mDestructor(storage, sizeof(AllocationPtr));
            static_cast<VariantStorage *>(storage)->mIndex.reset();
        }

        static constexpr Reflect::ExtendedType type(std::vector<const StorageOps *> &typeList)
        {
            assert(typeList.size() == 2);
            return { Reflect::ExtendedTypeEnum::VariantType, { typeList[0]->mType, typeList[1]->mType } };
        }

        static Serialize::StreamResult sRead(const StorageOps &ops, Serialize::CallerHierarchyFormattedSerializeStream in, BaseStorage &storage, const char *name, size_t inlineSize)
        {
            assert(inlineSize > 0);
            assert(inlineSize >= sizeof(AllocationPtr) + sizeof(uintptr_t));

            uint32_t newIndex;
            STREAM_PROPAGATE_ERROR(in.mStream.beginExtendedRead(name, 1));

            STREAM_PROPAGATE_ERROR(Serialize::read(in, newIndex, "type"));

            VariantStorage &vStorage = static_cast<VariantStorage &>(storage);
            const VariantStorageOps &vOps = static_cast<const VariantStorageOps &>(ops);

            if (newIndex != vStorage.mIndex - 1) {
                if (vStorage.mIndex) {
                    vOps.mTypeList[vStorage.mIndex - 1]->mDestructor(&storage, sizeof(AllocationPtr));
                    vStorage.mIndex.reset();
                }
                Reflect::Result result = vOps.mTypeList[newIndex]->construct(storage, {}, sizeof(AllocationPtr));
                if (result) {
                    return STREAM_UNKNOWN_ERROR() << "Construction of type " << vOps.mTypeList[newIndex]->mTypeName << " failed with: " << result;
                }
                vStorage.mIndex = newIndex + 1;
            }

            return vOps.mTypeList[vStorage.mIndex - 1]->read(in, storage, name, sizeof(AllocationPtr));
        }

        static void sWrite(Serialize::CallerHierarchyFormattedSerializeStream out, const BaseStorage &storage, const char *name, size_t inlineSize)
        {
            assert(inlineSize > 0);
            assert(inlineSize >= sizeof(AllocationPtr) + sizeof(uintptr_t));

            const VariantStorage &vStorage = static_cast<const VariantStorage &>(storage);
            const VariantStorageOps &vOps = static_cast<const VariantStorageOps &>(storage.mType);

            out.mStream.beginExtendedWrite(name, 1);
            Serialize::write(out, vStorage.mIndex - 1, "type");

            return vOps.mTypeList[vStorage.mIndex - 1]->mWrite(out, storage, name, sizeof(AllocationPtr));
        }

        static Serialize::StreamResult sApplyMap(BaseStorage &storage, Serialize::CallerHierarchyFormattedSerializeStream in, bool success, size_t inlineSize)
        {
            assert(inlineSize > 0);
            assert(inlineSize >= sizeof(AllocationPtr) + sizeof(uintptr_t));

            return static_cast<const VariantStorageOps &>(storage.mType).mTypeList[static_cast<VariantStorage &>(storage).mIndex - 1]->mApplyMap(storage, in, success, sizeof(AllocationPtr));
        }

        static constexpr Constructor sConstructors[2] { { sMatcher, sConstructor }, {} };

        explicit constexpr VariantStorageOps(std::vector<const StorageOps *> typeList)
            : StorageOps(&mSelfStorage, "TODO", sConstructors, &sMoveAssign, &sToValue, &sFromValue, type(typeList), &sDestructor, &sRead, &sWrite, &sApplyMap)
            , mTypeList(typeList)
            , mSelfStorage(this)
        {
        }

        std::vector<const StorageOps *> mTypeList;
        const StorageOps *mSelfStorage;
    };

    struct VariantStorageOpsComparator {

        using is_transparent = void;

        bool operator()(const VariantStorageOps &first, const VariantStorageOps &second) const
        {
            return first.mTypeList < second.mTypeList;
        }

        bool operator()(const VariantStorageOps &first, const std::vector<const StorageOps *> &second) const
        {
            return first.mTypeList < second;
        }

        bool operator()(const std::vector<const StorageOps *> &first, const VariantStorageOps &second) const
        {
            return first < second.mTypeList;
        }
    };

    const StorageOps &resolveVariantStorageOps(const std::vector<const StorageOps *> &types)
    {
        static std::set<VariantStorageOps, VariantStorageOpsComparator> sOps;
        auto it = sOps.find(types);
        if (it == sOps.end()) {
            it = sOps.emplace(types).first;
        }
        return *it;
    }

}
}