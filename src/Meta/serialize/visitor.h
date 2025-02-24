#pragma once

#include "hierarchy/serializableunitptr.h"
//#include "hierarchy/syncableunit.h"
#include "streams/streamresult.h"
#include "primitivetypes.h"

namespace Engine {
namespace Serialize {

    template <typename T>
    META_EXPORT StreamResult visitSkipPrimitive(FormattedSerializeStream &in, const char *name);

    META_EXPORT StreamResult visitSkipEnum(const EnumMetaTable *table, FormattedSerializeStream &in, const char *name);
    META_EXPORT StreamResult visitSkipFlags(const EnumMetaTable *table, FormattedSerializeStream &in, const char *name);
    META_EXPORT StreamResult visitSyncableUnit(const SerializeTable *table, FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor);

    template <typename...>
    struct StreamVisitorBase {
        virtual StreamResult visit(PrimitiveHolder<DataTag>, FormattedSerializeStream &in, const char *name, std::span<std::string_view> tags) const = 0;
        virtual StreamResult visit(PrimitiveHolder<SyncableUnitBase>, FormattedSerializeStream &in, const char *name, std::span<std::string_view> tags) const = 0;
    };

    template <typename T, typename... Ty>
    struct StreamVisitorBase<T, Ty...> : StreamVisitorBase<Ty...> {
        virtual StreamResult visit(PrimitiveHolder<T>, FormattedSerializeStream &in, const char *name, std::span<std::string_view> tags) const = 0;
        using StreamVisitorBase<Ty...>::visit;
    };

    struct StreamVisitor : SerializePrimitives::instantiate<StreamVisitorBase> {
    };

    template <typename F, typename...>
    struct StreamVisitorImplHelper : StreamVisitor {
        StreamVisitorImplHelper(F &&f)
            : mF(std::forward<F>(f))
        {
        }

        StreamResult visit(PrimitiveHolder<DataTag> holder, FormattedSerializeStream &in, const char *name, std::span<std::string_view> tags) const override
        {
            if constexpr (requires {
                              mF(holder, in, name, tags);
                          }) {
                return mF(holder, in, name, tags);
            } else {
                return SerializableDataPtr::visitStream(holder.mTable, in, name, *this);                
            }
        }

        StreamResult visit(PrimitiveHolder<SyncableUnitBase> holder, FormattedSerializeStream &in, const char *name, std::span<std::string_view> tags) const override
        {
            if constexpr (requires {
                              mF(holder, in, name, tags);
                          }) {
                return mF(holder, in, name, tags);
            } else {
                return visitSyncableUnit(holder.mTable, in, name, *this);
            }
        }

        using StreamVisitor::visit;

        F mF;
    };

    template <typename F, typename T, typename... Ty>
    struct StreamVisitorImplHelper<F, T, Ty...> : StreamVisitorImplHelper<F, Ty...> {
        using StreamVisitorImplHelper<F, Ty...>::StreamVisitorImplHelper;

        virtual StreamResult visit(PrimitiveHolder<T> holder, FormattedSerializeStream &in, const char *name, std::span<std::string_view> tags) const override
        {
            if constexpr (requires {
                              this->mF(holder, in, name, tags);
                          }) {
                return this->mF(holder, in, name, tags);
            } else if constexpr (std::same_as<T, EnumTag>) {
                return visitSkipEnum(holder.mTable, in, name);
            } else if constexpr (std::same_as<T, FlagsTag>) {
                return visitSkipFlags(holder.mTable, in, name);
            } else {
                return visitSkipPrimitive<T>(in, name);
            }
        }

        using StreamVisitorImplHelper<F, Ty...>::visit;
    };

    template <typename F>
    struct StreamVisitorImpl : SerializePrimitives::template prepend<F>::template instantiate<StreamVisitorImplHelper> {
        using SerializePrimitives::template prepend<F>::template instantiate<StreamVisitorImplHelper>::instantiate;
    };

    template <typename F>
    StreamVisitorImpl(F &&) -> StreamVisitorImpl<F>;

}
}