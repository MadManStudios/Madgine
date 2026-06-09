#pragma once

#include "serializestreamdata.h"

namespace Engine {
namespace Serialize {

    struct META_EXPORT SyncStreamData : SerializeStreamData {
    public:
        SyncStreamData(SyncManager &mgr, ParticipantId id = 0);
        SyncStreamData(const SyncStreamData &) = delete;
        SyncStreamData(SyncStreamData &&) noexcept = delete;

        virtual ~SyncStreamData() = default;

        SyncManager *manager();
    };

}
}