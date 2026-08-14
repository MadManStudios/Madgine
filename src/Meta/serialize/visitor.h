#pragma once

#include "hierarchy/serializableunitptr.h"
// #include "hierarchy/syncableunit.h"
#include "primitivetypes.h"
#include "streams/streamresult.h"

namespace Engine {
namespace Serialize {

    template <typename T>
    META_EXPORT StreamResult visitSkipPrimitive(PrimitiveHolder<T>, FormattedSerializeStream &in, const char *name);

    META_EXPORT StreamResult visitSkipEnum(const EnumMetaTable *table, FormattedSerializeStream &in, const char *name);
    META_EXPORT StreamResult visitSkipFlags(const EnumMetaTable *table, FormattedSerializeStream &in, const char *name);
    META_EXPORT StreamResult visitSyncableUnit(const SerializeTable *table, FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth);

    template <typename...>
    struct StreamVisitorBase {
        virtual StreamResult visit(PrimitiveHolder<DataTag>, FormattedSerializeStream &in, const char *name, std::span<std::string_view> tags, size_t depth) const = 0;
        virtual StreamResult visit(PrimitiveHolder<SyncableUnitBase>, FormattedSerializeStream &in, const char *name, std::span<std::string_view> tags, size_t depth) const = 0;
    };

    template <typename T, typename... Ty>
    struct StreamVisitorBase<T, Ty...> : StreamVisitorBase<Ty...> {
        virtual StreamResult visit(PrimitiveHolder<T>, FormattedSerializeStream &in, const char *name, std::span<std::string_view> tags, size_t depth) const = 0;
        using StreamVisitorBase<Ty...>::visit;
    };

    struct StreamVisitor : SerializePrimitives::instantiate<StreamVisitorBase> {
    };

    template <typename F, typename T>
    concept StreamVisitable = std::invocable<F, PrimitiveHolder<T>, FormattedSerializeStream &, const char *, std::span<std::string_view>, size_t>;

    template <typename F, typename...>
    struct StreamVisitorImplHelper : StreamVisitor {
        StreamVisitorImplHelper(F &&f)
            : mF(std::forward<F>(f))
        {
        }

        StreamResult visit(PrimitiveHolder<DataTag> holder, FormattedSerializeStream &in, const char *name, std::span<std::string_view> tags, size_t depth) const override
        {
            if constexpr (StreamVisitable<F, DataTag>) {
                std::optional<StreamResult> result = mF(holder, in, name, tags, depth);
                if (result)
                    return std::move(*result);
            }
            return SerializableDataPtr::visitStream(holder.mTable, in, name, *this, depth);
        }

        StreamResult visit(PrimitiveHolder<SyncableUnitBase> holder, FormattedSerializeStream &in, const char *name, std::span<std::string_view> tags, size_t depth) const override
        {
            if constexpr (StreamVisitable<F, SyncableUnitBase>) {
                std::optional<StreamResult> result = mF(holder, in, name, tags, depth);
                if (result)
                    return std::move(*result);
            }
            return visitSyncableUnit(holder.mTable, in, name, *this, depth);
        }

        using StreamVisitor::visit;

        F mF;
    };

    template <typename F, typename T, typename... Ty>
    struct StreamVisitorImplHelper<F, T, Ty...> : StreamVisitorImplHelper<F, Ty...> {
        using StreamVisitorImplHelper<F, Ty...>::StreamVisitorImplHelper;

        virtual StreamResult visit(PrimitiveHolder<T> holder, FormattedSerializeStream &in, const char *name, std::span<std::string_view> tags, size_t depth) const override
        {
            if constexpr (requires {
                              this->mF(holder, in, name, tags, depth);
                          }) {
                std::optional<StreamResult> result = this->mF(holder, in, name, tags, depth);
                if (result)
                    return std::move(*result);
            }
            return visitSkipPrimitive<T>(holder, in, name);
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