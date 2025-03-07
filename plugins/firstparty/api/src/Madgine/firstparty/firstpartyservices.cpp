#include "../firstpartylib.h"

#include "firstpartyservices.h"

#include "Madgine/root/root.h"

#include "Meta/keyvalue/metatable_impl.h"

METATABLE_BEGIN(Engine::FirstParty::FirstPartyServices)
READONLY_PROPERTY(Initialized, mInitialized)
METATABLE_END(Engine::FirstParty::FirstPartyServices)

VIRTUALUNIQUECOMPONENTBASE(Engine::FirstParty::FirstPartyServices)

namespace Engine {
namespace FirstParty {

    FirstPartyServices::FirstPartyServices(Root::Root &root)
        : VirtualRootComponentBase(root)
    {
    }

    bool FirstPartyServices::isConnected()
    {
        return is_instantiated() && getSingleton().mInitialized;
    }

    Threading::TaskFuture<Leaderboard> FirstPartyServices::getLeaderboard(const char *name, Leaderboard::AccessMode accessmode, Leaderboard::ReferenceRank referenceRank, int32_t rangeBegin, int32_t rangeEnd)
    {
        return mRoot.taskQueue()->queueTask(getLeaderboardTask(name, accessmode, referenceRank, rangeBegin, rangeEnd));
    }

    Threading::TaskFuture<bool> FirstPartyServices::ingestStat(const char *name, const char *leaderboardName, int32_t value)
    {
        return mRoot.taskQueue()->queueTask(ingestStatTask(name, leaderboardName, value));
    }

    Threading::TaskFuture<bool> FirstPartyServices::unlockAchievement(const char *name)
    {
        return mRoot.taskQueue()->queueTask(unlockAchievementTask(name));
    }

    Threading::TaskFuture<std::vector<Lobby>> FirstPartyServices::getLobbyList()
    {
        return mRoot.taskQueue()->queueTask(getLobbyListTask());
    }

    Threading::TaskFuture<std::optional<Lobby>> FirstPartyServices::createLobby(size_t maxPlayerCount, MatchmakingCallback cb, SessionStartedCallback sessionCb, std::map<std::string, std::string> properties)
    {
        return mRoot.taskQueue()->queueTask(createLobbyTask(maxPlayerCount, std::move(cb), std::move(sessionCb), std::move(properties)));
    }

    Threading::TaskFuture<std::optional<Lobby>> FirstPartyServices::joinLobby(uint64_t id, MatchmakingCallback cb, SessionStartedCallback sessionCb)
    {
        return mRoot.taskQueue()->queueTask(joinLobbyTask(id, std::move(cb), std::move(sessionCb)));
    }

    Threading::TaskFuture<std::optional<ServerInfo>> FirstPartyServices::startMatch()
    {
        return mRoot.taskQueue()->queueTask(startMatchTask());
    }

    Execution::ValueStub<std::optional<LobbyInfo>> &FirstPartyServices::lobbyInfo()
    {
        return mLobbyInfo;
    }

}
}
