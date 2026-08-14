#pragma once

#include "Generic/linestruct.h"

#include "serializableunitptr.h"

namespace Engine {
namespace Serialize {

#define SERIALIZABLEUNIT_MEMBERS() \
    READONLY_PROPERTY(Synced, isSynced)

#define SERIALIZABLEUNIT(_Self)                                 \
    template <typename Tag, size_t...>                          \
    friend struct ::Engine::__generic_impl__::LineStruct;       \
    friend struct ::Engine::Serialize::SerializeTableCallbacks; \
    using Self = _Self;

    struct META_EXPORT SerializableUnitBase {
    protected:
        SerializableUnitBase();
        SerializableUnitBase(const SerializableUnitBase &other);
        SerializableUnitBase(SerializableUnitBase &&other) noexcept;
        ~SerializableUnitBase();

        SerializableUnitBase &operator=(const SerializableUnitBase &other);
        SerializableUnitBase &operator=(SerializableUnitBase &&other);

    public:
        bool isSynced() const;

    protected:
        friend struct SyncableBase;
        friend struct SerializableUnitPtr;
        friend struct SerializableUnitConstPtr;
        friend struct TopLevelUnitBase;
        friend struct SerializeTable;
        template <typename>
        friend struct Serializable;
        friend struct SyncableUnitBase;
        friend struct SyncManager;

        template <std::derived_from<SerializableUnitBase> T>
        friend void tag_invoke(const set_parent_t &, T &unit, SerializableUnitBase *parent)
        {
            SerializableUnitPtr { &unit }.setParent(parent);
        }

        template <std::derived_from<SerializableUnitBase> T, typename Context>
        friend void tag_invoke(const set_synced_t &, T &unit, bool b, Context &&context)
        {
            SerializableUnitPtr { &unit }.setSynced(b, context);
        }

        template <std::derived_from<SerializableUnitBase> T, typename... Configs, typename Context>
        friend void tag_invoke(const set_active_t<Configs...> &, T &unit, bool active, bool existenceChanged, Context &&context)
        {
            SerializableUnitPtr { &unit }.setActive(active, existenceChanged, context);
        }

    private:
        const TopLevelUnitBase *mTopLevel = nullptr;

        uint8_t mActiveIndex = 0;

        bool mSynced = false;
    };

} // namespace Serialize
} // namespace Core
