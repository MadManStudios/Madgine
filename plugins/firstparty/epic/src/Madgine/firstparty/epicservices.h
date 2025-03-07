#pragma once

#include "Madgine/firstparty/firstpartyservices.h"

#include "Modules/threading/taskqueue.h"

#include <eos_auth_types.h>
#include <eos_connect_types.h>
#include <eos_leaderboards_types.h>
#include <eos_stats_types.h>
#include <eos_types.h>

namespace Engine {
namespace FirstParty {

    struct EpicServices : FirstPartyServicesImpl<EpicServices> {

        EpicServices(Root::Root &root);
        ~EpicServices();

        Threading::Task<EOS_EpicAccountId> loginUser();
        Threading::Task<EOS_ProductUserId> connectLogin(EOS_EpicAccountId id);

        std::string_view key() const override;

        ////////// IDENTITY

        std::string currentUserName() const override;

        ////////// LEADERBOARD

        struct LeaderboardInfo {
            std::string mId;
            std::string mStatName;
            enum {
                MIN,
                MAX,
                SUM,
                LATEST
            } mAggregation;
        };

        Threading::Task<Leaderboard> getLeaderboardTask(const char *name, Leaderboard::AccessMode accessmode, Leaderboard::ReferenceRank referenceRank, int32_t rangeBegin, int32_t rangeEnd) override;
        Threading::Task<std::vector<LeaderboardInfo>> getLeaderboards();

        struct LeaderboardCache {
            Leaderboard mBoard;
            std::chrono::steady_clock::time_point mLastUpdate;
        };
        struct LeaderboardDescriptor {
            std::string mName;
            Leaderboard::AccessMode mAccessMode;

            auto operator<=>(const LeaderboardDescriptor &) const = default;
        };
        std::map<LeaderboardDescriptor, LeaderboardCache> mCache;

        /////////// STATS

        Threading::Task<bool> ingestStatTask(const char *name, const char *leaderboardName, int32_t value) override;

        /////////// ACHIEVEMENTS

        Threading::Task<bool> unlockAchievementTask(const char *name) override;

        /////////// MATCHMAKING

        Threading::Task<std::vector<Lobby>> getLobbyListTask() override;
        Threading::Task<std::optional<Lobby>> createLobbyTask(size_t maxPlayerCount, MatchmakingCallback cb, SessionStartedCallback sessionCb, std::map<std::string, std::string> properties = {}) override;
        Threading::Task<std::optional<Lobby>> joinLobbyTask(uint64_t id, MatchmakingCallback cb, SessionStartedCallback sessionCb) override;
        Threading::Task<std::optional<ServerInfo>> startMatchTask() override;
        void leaveLobby() override;
        void leaveMatch() override;
        bool isLobbyOwner() const override;

        void setLobbyProperty(std::string_view key, std::string_view value) override;

        //////////////////

        EOS_HPlatform mPlatformHandle = nullptr;
        EOS_HAuth mAuthInterface = nullptr;
        EOS_HConnect mConnectInterface = nullptr;
        EOS_HLeaderboards mLeaderboardsInterface = nullptr;
        EOS_HStats mStatsInterface = nullptr;

#if EOS_LOGIN_METHOD_CREDENTIALS
        bool mRetryAccountOnFailure = false;
#endif

        Threading::TaskFuture<EOS_EpicAccountId> mUserId;
        Threading::TaskFuture<EOS_ProductUserId> mProductUserId;
    };

}
}

REGISTER_TYPE(Engine::FirstParty::EpicServices)