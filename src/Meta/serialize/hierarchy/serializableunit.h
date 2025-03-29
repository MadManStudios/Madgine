#pragma once

#include "../operations.h"

#include "Generic/linestruct.h"

namespace Engine {
namespace Serialize {

#define SERIALIZABLEUNIT_MEMBERS() \
    READONLY_PROPERTY(Synced, isSynced)

    
#define SERIALIZABLEUNIT(_Self)                                 \
    template <typename Tag, size_t...>                          \
    friend struct ::Engine::LineStruct;                         \
    friend struct ::Engine::Serialize::SerializeTableCallbacks; \
    DERIVE_FRIEND(onActivate, ::Engine::Serialize::)            \
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
        friend void tag_invoke(set_parent_t, T &unit, SerializableUnitBase *parent)
        {
            SerializableUnitPtr { &unit }.setParent(parent);
        }
                
        template <std::derived_from<SerializableUnitBase> T>
        friend void tag_invoke(set_synced_t cpo, T &unit, bool b, const CallerHierarchyBasePtr &hierarchy)
        {
            SerializableUnitPtr { &unit }.setSynced(b, hierarchy);
        }

    private:
        const TopLevelUnitBase *mTopLevel = nullptr;

        uint8_t mActiveIndex = 0;

        bool mSynced = false;
    };

} // namespace Serialize
} // namespace Core
