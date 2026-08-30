#include "../metalib.h"

#include "typenames.h"

#include "../reflect/metatable.h"
#include "../serialize/hierarchy/serializetable.h"
#include "storageops.h"

namespace Engine {
namespace Type {

    std::map<std::string, TypeName, std::less<>> &mutableTypeList()
    {
        static std::map<std::string, TypeName, std::less<>> sMap;
        return sMap;
    }

    const std::map<std::string, TypeName, std::less<>> &typeList()
    {
        return mutableTypeList();
    }

    const TypeName *resolveTypeName(std::string_view name, std::string_view separator)
    {
        auto gen = StringUtil::tokenize(name, separator);

        auto it = mutableTypeList().find(gen.get());
        if (it == mutableTypeList().end())
            return nullptr;

        gen.next();

        TypeName *type = &it->second;

        for (std::string_view part : gen) {
            auto it = type->mMembers.find(part);
            if (it == type->mMembers.end())
                return nullptr;
            type = &it->second;
        }

        return type;
    }

    TypeName &addTypeName(std::string_view name)
    {
        auto gen = StringUtil::tokenize(name, "::");

        TypeName *type = &mutableTypeList()[std::string { gen.get() }];

        gen.next();

        for (std::string_view part : gen) {
            type = &type->mMembers[std::string { part }];
        }

        return *type;
    }

    void registerMetaTable(const Reflect::MetaTable &table)
    {
        addTypeName(table.mTypeName).mMetaTable = &table;
    }

    void unregisterMetaTable(const Reflect::MetaTable &table)
    {
        addTypeName(table.mTypeName).mMetaTable = nullptr;
    }

}
}