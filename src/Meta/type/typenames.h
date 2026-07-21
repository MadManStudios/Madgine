#pragma once

namespace Engine {
namespace Type {

    struct TypeName {
        std::map<std::string, TypeName, std::less<>> mMembers;

        const Reflect::MetaTable *mMetaTable = nullptr;
        const StorageOps *mStorageOps = nullptr;
    };

    META_EXPORT const TypeName *resolveTypeName(std::string_view name, std::string_view separator = "::");

    META_EXPORT const std::map<std::string, TypeName, std::less<>> &typeList();

    META_EXPORT void registerMetaTable(const Reflect::MetaTable &table);
    META_EXPORT void unregisterMetaTable(const Reflect::MetaTable &table);
    META_EXPORT void registerStorageOps(const Type::StorageOps &ops);
    META_EXPORT void unregisterStorageOps(const Type::StorageOps &ops);

}
}