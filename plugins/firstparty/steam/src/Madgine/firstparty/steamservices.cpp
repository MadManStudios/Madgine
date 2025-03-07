#include "../steamserviceslib.h"

#include "steamservices.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"

#include <steam/steam_api.h>

#include "steam_sender.h"

#include "Modules/threading/awaitables/awaitabletimepoint.h"

#include "Madgine/root/root.h"

#include "Modules/threading/awaitables/awaitablesender.h"

#include "Generic/execution/execution.h"

METATABLE_BEGIN_BASE(Engine::FirstParty::SteamServices, Engine::FirstParty::FirstPartyServices)
METATABLE_END(Engine::FirstParty::SteamServices)

UNIQUECOMPONENT(Engine::FirstParty::SteamServices)

namespace Engine {
namespace FirstParty {

    SteamServices::SteamServices(Root::Root &root)
        : FirstPartyServicesImpl<SteamServices>(root)
        , mSyncManager("SteamP2P")
    {
        if (SteamAPI_RestartAppIfNecessary(STEAM_APP_ID)) {
            mErrorCode = -1;
            return;
        }

        mInitialized = SteamAPI_Init();

        mSyncManager.setup();

        root.taskQueue()->queue([this]() -> Threading::Task<void> {
            while (mRoot.taskQueue()->running()) {
                SteamAPI_RunCallbacks();
                mSyncManager.sendAndReceiveMessages();
                co_await 10ms;
            }
        });
    }

    SteamServices::~SteamServices()
    {
        mSyncManager.disconnect();

        SteamAPI_Shutdown();
    }

    std::string_view SteamServices::key() const
    {
        return "SteamServices";
    }

    std::string SteamServices::currentUserName() const
    {
        return SteamFriends()->GetPersonaName();
    }

    Threading::Task<Leaderboard> SteamServices::getLeaderboardTask(const char *name, Leaderboard::AccessMode accessmode, Leaderboard::ReferenceRank referenceRank, int32_t rangeBegin, int32_t rangeEnd)
    {
        ELeaderboardDataRequest requestmode;
        switch (accessmode) {
        case Leaderboard::GLOBAL:
            switch (referenceRank) {
            case Leaderboard::FIRST:
                requestmode = k_ELeaderboardDataRequestGlobal;
                ++rangeBegin;
                ++rangeEnd;
                break;
            case Leaderboard::USER:
                requestmode = k_ELeaderboardDataRequestGlobalAroundUser;
                break;
            default:
                co_return {};
            }
            break;
        case Leaderboard::FRIENDS:
            requestmode = k_ELeaderboardDataRequestFriends;
            break;
        default:
            co_return {};
        }

        LeaderboardFindResult_t leaderboard = (co_await steam_sender<LeaderboardFindResult_t>(SteamUserStats()->FindLeaderboard(name))).value();
        if (!leaderboard.m_bLeaderboardFound)
            co_return {};

        LeaderboardScoresDownloaded_t download = (co_await steam_sender<LeaderboardScoresDownloaded_t>(SteamUserStats()->DownloadLeaderboardEntries(leaderboard.m_hSteamLeaderboard, requestmode, rangeBegin, rangeEnd))).value();

        Leaderboard result;
        for (size_t i = 0; i < download.m_cEntryCount; ++i) {
            Leaderboard::Entry &entry = result.mEntries.emplace_back();

            LeaderboardEntry_t leaderboardEntry;

            if (!SteamUserStats()->GetDownloadedLeaderboardEntry(download.m_hSteamLeaderboardEntries, i, &leaderboardEntry, nullptr, 0))
                co_return {};

            entry.mRank = leaderboardEntry.m_nGlobalRank;
            entry.mScore = leaderboardEntry.m_nScore;
            entry.mDisplayName = SteamFriends()->GetFriendPersonaName(leaderboardEntry.m_steamIDUser);
        }

        co_return result;
    }

    Threading::Task<bool> SteamServices::ingestStatTask(const char *name, const char *leaderboardName, int32_t value)
    {
        if (!SteamUserStats()->SetStat(name, value))
            co_return false;

        LeaderboardFindResult_t leaderboardFound = (co_await steam_sender<LeaderboardFindResult_t>(SteamUserStats()->FindLeaderboard(leaderboardName))).value();
        if (!leaderboardFound.m_bLeaderboardFound)
            co_return false;

        auto upload = steam_sender<LeaderboardScoreUploaded_t>(SteamUserStats()->UploadLeaderboardScore(leaderboardFound.m_hSteamLeaderboard, k_ELeaderboardUploadScoreMethodKeepBest, value, nullptr, 0));

        bool success = SteamUserStats()->StoreStats();

        LeaderboardScoreUploaded_t payload = (co_await std::move(upload)).value();

        co_return success &&payload.m_bSuccess;
    }

    Threading::Task<bool> SteamServices::unlockAchievementTask(const char *name)
    {
        if (!SteamUserStats()->SetAchievement(name))
            co_return false;

        co_return SteamUserStats()->StoreStats();
    }

    Threading::Task<std::vector<Lobby>> SteamServices::getLobbyListTask()
    {
        LobbyMatchList_t result = (co_await steam_sender<LobbyMatchList_t>(SteamMatchmaking()->RequestLobbyList())).value();

        std::vector<Lobby> lobbies;
        lobbies.reserve(result.m_nLobbiesMatching);

        for (size_t i = 0; i < result.m_nLobbiesMatching; ++i) {
            Lobby &lobby = lobbies.emplace_back();
            CSteamID id = SteamMatchmaking()->GetLobbyByIndex(i);
            lobby.mId = id.ConvertToUint64();
            lobby.mProperties = getLobbyProperties(id);
        }

        co_return std::move(lobbies);
    }

    Threading::Task<std::optional<Lobby>> SteamServices::createLobbyTask(size_t maxPlayerCount, MatchmakingCallback cb, SessionStartedCallback sessionCb, std::map<std::string, std::string> properties)
    {
        assert(!mCurrentLobby.IsValid());

        LobbyCreated_t result = (co_await steam_sender<LobbyCreated_t>(SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, maxPlayerCount))).value();

        if (result.m_eResult != k_EResultOK) {
            LOG_ERROR("Error creating Steam Lobby: " << result.m_eResult);
            co_return {};
        }

        mCurrentMatchmakingCallback = std::move(cb);
        mSessionStartedCallback = std::move(sessionCb);
        mCurrentLobby.SetFromUint64(result.m_ulSteamIDLobby);

        for (const auto &[key, value] : properties)
            SteamMatchmaking()->SetLobbyData(mCurrentLobby, key.c_str(), value.c_str());

        updateLobbyInfo();

        co_return Lobby { result.m_ulSteamIDLobby };
    }

    Threading::Task<std::optional<Lobby>> SteamServices::joinLobbyTask(uint64_t id, MatchmakingCallback cb, SessionStartedCallback sessionCb)
    {
        assert(!mCurrentLobby.IsValid());

        LobbyEnter_t result = (co_await steam_sender<LobbyEnter_t>(SteamMatchmaking()->JoinLobby(id))).value();

        if (result.m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess) {
            LOG_ERROR("Error joining Steam Lobby(" << id << "): " << result.m_EChatRoomEnterResponse);
            co_return {};
        }

        mCurrentMatchmakingCallback = std::move(cb);
        mSessionStartedCallback = std::move(sessionCb);
        mCurrentLobby.SetFromUint64(result.m_ulSteamIDLobby);

        updateLobbyInfo();

        co_return Lobby { result.m_ulSteamIDLobby };
    }

    Threading::Task<std::optional<ServerInfo>> SteamServices::startMatchTask()
    {
        if (!mCurrentLobby.IsValid() || !isLobbyOwner()) {
            co_return {};
        }

        int playerCount = SteamMatchmaking()->GetNumLobbyMembers(mCurrentLobby);

        ServerInfo server;
        server.mSession.mPlayers = getLobbyPlayers();
        std::vector<CSteamID> players;
        for (int i = 0; i < playerCount; ++i) {
            players.emplace_back(SteamMatchmaking()->GetLobbyMemberByIndex(mCurrentLobby, i));
        }

        Serialize::Format format = mCurrentMatchmakingCallback(mSyncManager);
        mSyncManager.listen(playerCount - 1, format);

        SteamMatchmaking()->SetLobbyGameServer(mCurrentLobby, 0, 0, SteamUser()->GetSteamID());

        if (mSessionStartedCallback)
            mSessionStartedCallback(server.mSession);

        if (playerCount > 1)
            co_await mSyncManager.playersConnected();

        leaveLobby();

        for (size_t i = 0; i < playerCount; ++i) {
            server.mIds.emplace_back(mSyncManager.resolvePlayerId(players[i]));
        }

        co_return std::move(server);
    }

    void SteamServices::leaveLobby()
    {
        if (mCurrentLobby.IsValid()) {
            SteamMatchmaking()->LeaveLobby(mCurrentLobby);
            mCurrentLobby.Clear();
            mCurrentMatchmakingCallback = {};
            mSessionStartedCallback = {};
        } else {
            LOG_WARNING("Trying to leave Lobby without being in one");
        }
    }

    void SteamServices::leaveMatch()
    {
        mSyncManager.disconnect();
    }

    bool SteamServices::isLobbyOwner() const
    {
        if (!mCurrentLobby.IsValid())
            return false;

        return SteamUser()->GetSteamID() == SteamMatchmaking()->GetLobbyOwner(mCurrentLobby);
    }

    void SteamServices::setLobbyProperty(std::string_view key, std::string_view value)
    {
        if (!mCurrentLobby.IsValid())
            return;

        SteamMatchmaking()->SetLobbyData(mCurrentLobby, key.data(), value.data());
    }

    void SteamServices::updateLobbyInfo()
    {
        if (mCurrentLobby.IsValid()) {
            LobbyInfo lobbyInfo;
            lobbyInfo.mPlayers = getLobbyPlayers();
            lobbyInfo.mProperties = getLobbyProperties(mCurrentLobby);
            mLobbyInfo.set(std::move(lobbyInfo));

            // if (mLobbyRunning != running) {
            //     onMatchStarted();
            // }
        }
    }

    void SteamServices::onMatchStarted(CSteamID serverID)
    {
        if (serverID != SteamUser()->GetSteamID()) {
            Serialize::Format format = mCurrentMatchmakingCallback(mSyncManager);

            SteamMatchmaking()->GetLobbyGameServer(mCurrentLobby, nullptr, nullptr, &serverID);

            Execution::detach(mSyncManager.connect(serverID, format) | Execution::then([this]() {
                if (mSessionStartedCallback)
                    mSessionStartedCallback({ getLobbyPlayers() });

                leaveLobby();
            }));
        }
    }

    std::map<std::string, std::string> SteamServices::getLobbyProperties(CSteamID lobby)
    {
        int dataCount = SteamMatchmaking()->GetLobbyDataCount(lobby);

        std::map<std::string, std::string> properties;
        for (int i = 0; i < dataCount; ++i) {
            char key[256];
            char value[256];
            bool result = SteamMatchmaking()->GetLobbyDataByIndex(lobby, i, key, 255, value, 255);
            if (result)
                properties[key] = value;
        }
        return properties;
    }

    std::vector<PlayerInfo> SteamServices::getLobbyPlayers()
    {
        int playerCount = SteamMatchmaking()->GetNumLobbyMembers(mCurrentLobby);

        std::vector<PlayerInfo> players;
        for (int i = 0; i < playerCount; ++i) {
            CSteamID user = SteamMatchmaking()->GetLobbyMemberByIndex(mCurrentLobby, i);
            PlayerInfo &info = players.emplace_back();
            info.mName = SteamFriends()->GetFriendPersonaName(user);
        }
        return players;
    }

    void SteamServices::onLobbyInfoUpdate(LobbyDataUpdate_t *data)
    {
        updateLobbyInfo();
    }

    void SteamServices::onChatUpdate(LobbyChatUpdate_t *data)
    {
        updateLobbyInfo();
    }

    void SteamServices::onGameCreated(LobbyGameCreated_t *data)
    {
        onMatchStarted(data->m_ulSteamIDGameServer);
    }

}
}
