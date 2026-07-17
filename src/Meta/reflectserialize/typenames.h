#pragma once

namespace Engine {

struct TypeName {

    std::map<std::string, TypeName, std::less<>> mMembers;
    const Serialize::SerializeTable *mSerializeTable = nullptr;
    const Reflect::MetaTable *mMetaTable = nullptr;
};

META_EXPORT const TypeName *resolveTypeName(std::string_view name, std::string_view separator = "::");

META_EXPORT const std::map<std::string, TypeName, std::less<>> &typeList();

META_EXPORT void registerSerializeTable(const Serialize::SerializeTable &table);
META_EXPORT void unregisterSerializeTable(const Serialize::SerializeTable &table);
META_EXPORT void registerMetaTable(const Reflect::MetaTable &table);
META_EXPORT void unregisterMetaTable(const Reflect::MetaTable &table);


}