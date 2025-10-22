#pragma once

#include "hierarchy/serializableunitptr.h"
// #include "hierarchy/syncableunit.h"
#include "primitivetypes.h"
#include "streams/streamresult.h"

namespace Engine {
namespace Serialize {

    template <typename T>
    META_EXPORT StreamResult visitSkipPrimitive(CallerHierarchyFormattedSerializeStream in, const char *name);

    META_EXPORT StreamResult visitSkipEnum(const EnumMetaTable *table, CallerHierarchyFormattedSerializeStream in, const char *name);
    META_EXPORT StreamResult visitSkipFlags(const EnumMetaTable *table, CallerHierarchyFormattedSerializeStream in, const char *name);
    META_EXPORT StreamResult visitSyncableUnit(const SerializeTable *table, CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth);

    template <typename...>
    struct StreamVisitorBase {
        virtual StreamResult visit(PrimitiveHolder<DataTag>, CallerHierarchyFormattedSerializeStream in, const char *name, std::span<std::string_view> tags, size_t depth) const = 0;
        virtual StreamResult visit(PrimitiveHolder<SyncableUnitBase>, CallerHierarchyFormattedSerializeStream in, const char *name, std::span<std::string_view> tags, size_t depth) const = 0;
    };

    template <typename T, typename... Ty>
    struct StreamVisitorBase<T, Ty...> : StreamVisitorBase<Ty...> {
        virtual StreamResult visit(PrimitiveHolder<T>, CallerHierarchyFormattedSerializeStream in, const char *name, std::span<std::string_view> tags, size_t depth) const = 0;
        using StreamVisitorBase<Ty...>::visit;
    };

    struct StreamVisitor : SerializePrimitives::instantiate<StreamVisitorBase> {
    };

    template <typename F, typename T>
    concept StreamVisitable = std::invocable<F, PrimitiveHolder<T>, CallerHierarchyFormattedSerializeStream, const char *, std::span<std::string_view>, size_t>;

    template <typename F, typename...>
    struct StreamVisitorImplHelper : StreamVisitor {
        StreamVisitorImplHelper(F &&f)
            : mF(std::forward<F>(f))
        {
        }

        StreamResult visit(PrimitiveHolder<DataTag> holder, CallerHierarchyFormattedSerializeStream in, const char *name, std::span<std::string_view> tags, size_t depth) const override
        {
            if constexpr (StreamVisitable<F, DataTag>) {
                std::optional<StreamResult> result = mF(holder, in, name, tags, depth);
                if (result)
                    return std::move(*result);
            }
            return SerializableDataPtr::visitStream(holder.mTable, in, name, *this, depth);
        }

        StreamResult visit(PrimitiveHolder<SyncableUnitBase> holder, CallerHierarchyFormattedSerializeStream in, const char *name, std::span<std::string_view> tags, size_t depth) const override
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

        virtual StreamResult visit(PrimitiveHolder<T> holder, CallerHierarchyFormattedSerializeStream in, const char *name, std::span<std::string_view> tags, size_t depth) const override
        {
            if constexpr (requires {
                              this->mF(holder, in, name, tags, depth);
                          }) {
                std::optional<StreamResult> result = this->mF(holder, in, name, tags, depth);
                if (result)
                    return std::move(*result);
            } 
            if constexpr (std::same_as<T, EnumTag>) {
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