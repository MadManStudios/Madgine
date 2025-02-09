#include "../mockupserviceslib.h"

#include "mockupservices.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Modules/threading/awaitables/awaitabletimepoint.h"

#include "Madgine/root/root.h"

#include "Modules/threading/awaitables/awaitablesender.h"

#include "Generic/execution/execution.h"

#include "Meta/serialize/formats.h"

METATABLE_BEGIN_BASE(Engine::FirstParty::MockupServices, Engine::FirstParty::FirstPartyServices)
READONLY_PROPERTY(Initialized, mInitialized)
METATABLE_END(Engine::FirstParty::MockupServices)

UNIQUECOMPONENT(Engine::FirstParty::MockupServices)

namespace Engine {
namespace FirstParty {

    MockupClientState::MockupClientState(MockupServices &services)
        : mServices(services)
    {
    }

    void MockupClientState::update()
    {
        MockupState::update();
    }

    Threading::Task<bool> MockupClientState::connect()
    {
        co_return (co_await mNetwork.connect("127.0.0.1", 12346, Serialize::Formats::xml, 1s)).is_value();
    }

    void MockupClientState::updateLobbyInfoImpl(LobbyInfo info)
    {
        mServices.updateLobbyInfo(std::move(info));
    }

    void MockupClientState::sendServerAddressImpl(SocketAddress address, std::vector<PlayerInfo> players)
    {
        mServices.connectToServer(std::move(address), std::move(players));
    }

    MockupServices::MockupServices(Root::Root &root)
        : FirstPartyServicesImpl<MockupServices>(root)
        , mSyncManager("MockupP2P")
        , mState { *this }
    {

        root.taskQueue()->queue([this]() -> Threading::Task<void> {
            while (mRoot.taskQueue()->running()) {
                mSyncManager.sendAndReceiveMessages();
                mState.update();
                co_await 10ms;
            }
        });
        root.taskQueue()->queue([this]() -> Threading::Task<void> {
            mInitialized = co_await mState.connect();
        });
    }

    MockupServices::~MockupServices()
    {
    }

    std::string_view MockupServices::key() const
    {
        return "MockupServices";
    }

    std::string MockupServices::currentUserName() const
    {
        return "Me";
    }

    Threading::Task<Leaderboard> MockupServices::getLeaderboardTask(const char *name, Leaderboard::AccessMode accessmode, Leaderboard::ReferenceRank referenceRank, int32_t rangeBegin, int32_t rangeEnd)
    {
        co_return { { { 1, "Bob", 1, 1000 } } };
    }

    Threading::Task<bool> MockupServices::ingestStatTask(const char *name, const char *leaderboardName, int32_t value)
    {
        co_return co_await mState.ingestStat(name, leaderboardName, value);
    }

    Threading::Task<bool> MockupServices::unlockAchievementTask(const char *name)
    {
        co_return co_await mState.unlockAchievement(name);
    }

    Threading::Task<std::vector<Lobby>> MockupServices::getLobbyListTask()
    {
        co_return co_await mState.getLobbyList();
    }

    Threading::Task<std::optional<Lobby>> MockupServices::createLobbyTask(MatchmakingCallback cb, SessionStartedCallback sessionCb, std::map<std::string, std::string> properties)
    {
        std::optional<Lobby> lobby = co_await mState.createLobby(properties);

        if (lobby) {
            mCurrentMatchmakingCallback = std::move(cb);
            mSessionStartedCallback = std::move(sessionCb);
        }

        co_return lobby;
    }

    Threading::Task<std::optional<Lobby>> MockupServices::joinLobbyTask(uint64_t id, MatchmakingCallback cb, SessionStartedCallback sessionCb)
    {
        std::optional<Lobby> lobby = co_await mState.joinLobby(id);

        if (lobby) {
            mCurrentMatchmakingCallback = std::move(cb);
            mSessionStartedCallback = std::move(sessionCb);
        }

        co_return lobby;
    }

    Threading::Task<ServerInfo> MockupServices::startMatchTask()
    {
        Serialize::Format format = mCurrentMatchmakingCallback(mSyncManager);
        mSyncManager.startServer(12348);

        std::vector<PlayerInfo> players = co_await mState.startMatch();

        if (mSessionStartedCallback)
            mSessionStartedCallback(players);

        size_t playerCount = players.size();

        while (mSyncManager.clientCount() < playerCount - 1) {
            mSyncManager.acceptConnections(format, playerCount - 1 - mSyncManager.clientCount());
            co_await 100ms;
        }

        leaveLobby();

        std::set<Serialize::ParticipantId> clients = mSyncManager.clients();
        clients.insert(Serialize::sLocalMasterParticipantId);

        co_return {
            players,
            { clients.begin(), clients.end() }
        };
    }

    void MockupServices::leaveLobby()
    {
        mState.leaveLobby();
    }

    void MockupServices::leaveMatch()
    {
        mState.leaveMatch();
    }

    bool MockupServices::isLobbyOwner() const
    {
        return true;
    }

    void MockupServices::setLobbyProperty(std::string_view key, std::string_view value)
    {
        mState.setLobbyProperty(key, value);
    }

    void MockupServices::updateLobbyInfo(LobbyInfo info)
    {
        mLobbyInfo.set(std::move(info));
    }

    void MockupServices::connectToServer(SocketAddress address, std::vector<PlayerInfo> players)
    {
        Serialize::Format format = mCurrentMatchmakingCallback(mSyncManager);

        Execution::detach(mSyncManager.connect(address, 12348, format) | Execution::then([this, players { std::move(players) }]() mutable {
            if (mSessionStartedCallback)
                mSessionStartedCallback(std::move(players));

            leaveLobby();
        }));
    }

}
}
