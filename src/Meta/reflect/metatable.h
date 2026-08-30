#pragma once

#include "Generic/offsetptr.h"

#include "../meta_decay.h"
#include "../type/typenames.h"
#include "result.h"
#include "table_forward.h"

namespace Engine {
namespace Reflect {

    template <typename T>
        requires(!std::is_pointer_v<T>)
    T *scope_cast(const ScopePtr &ptr);

    struct META_EXPORT MetaTable {

        constexpr MetaTable(const MetaTable **self, const char *name, const Accessor *members, const Engine::Type::StorageOps **storage)
            : mSelf(self)
            , mTypeName(name)
            , mBase(nullptr)
            , mBaseOffset(nullptr)
            , mMembers(members)
            , mStorage(storage)
        {
        }

        template <typename T, typename Base>
        constexpr MetaTable(const char *name, type_holder_t<T>, type_holder_t<Base>, const Accessor *members, const Engine::Type::StorageOps **storage)
            : mSelf(&table<T>)
            , mTypeName(name)
            , mBase(&table<Base>)
            , mBaseOffset([]() { return OffsetPtr { type_holder<T>, type_holder<Base> }; })
            , mMembers(members)
            , mStorage(storage)
        {
        }

        template <typename T>
        constexpr MetaTable(const char *name, type_holder_t<T>, type_holder_t<void>, const Accessor *members, const Engine::Type::StorageOps **storage)
            : mSelf(&table<T>)
            , mTypeName(name)
            , mBase(nullptr)
            , mBaseOffset(nullptr)
            , mMembers(members)
            , mStorage(storage)
        {
        }

        ScopeIterator find(std::string_view key, const Value &scope) const;

        Result call(const Value &scope, Value &retVal, const ArgumentList &args) const;

        template <typename T>
        bool isDerivedFrom(OffsetPtr *offset = nullptr) const
        {
            return isDerivedFrom(table<meta_decayed_t<T>>, offset);
        }
        bool isDerivedFrom(const MetaTable *baseType, OffsetPtr *offset = nullptr) const;

        std::string name(const Value &scope) const;

        const MetaTable **mSelf;
        const char *mTypeName;
        const MetaTable **mBase;
        OffsetPtr (*mBaseOffset)();
        const Accessor *mMembers;
        const Engine::Type::StorageOps **mStorage;
    };

    namespace __Reflect_impl__ {

        template <typename T>
        struct MetaTableRegistrator {
            MetaTableRegistrator()
            {
                Engine::Type::registerMetaTable(*table<T>);
            }
            ~MetaTableRegistrator()
            {
                Engine::Type::unregisterMetaTable(*table<T>);
            }
        };

    }
}
}