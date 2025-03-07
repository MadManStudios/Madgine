#include "../mockupliblib.h"
#include "mockupstate.h"

#include "Meta/serialize/serializetable_impl.h"

#include "Modules/threading/task.h"

#include "Modules/threading/awaitables/awaitablesender.h"

SERIALIZETABLE_BEGIN(Engine::FirstParty::Lobby)
FIELD(mId)
FIELD(mProperties)
SERIALIZETABLE_END(Engine::FirstParty::Lobby)

SERIALIZETABLE_BEGIN(Engine::FirstParty::LobbyInfo)
FIELD(mPlayers)
FIELD(mProperties)
SERIALIZETABLE_END(Engine::FirstParty::LobbyInfo)

SERIALIZETABLE_BEGIN(Engine::FirstParty::PlayerInfo)
FIELD(mName)
SERIALIZETABLE_END(Engine::FirstParty::PlayerInfo)

SERIALIZETABLE_BEGIN(Engine::FirstParty::ServerInfo)
FIELD(mSession)
FIELD(mIds)
SERIALIZETABLE_END(Engine::FirstParty::ServerInfo)

SERIALIZETABLE_BEGIN(Engine::FirstParty::SessionInfo)
FIELD(mPlayers)
SERIALIZETABLE_END(Engine::FirstParty::SessionInfo)

SERIALIZETABLE_BEGIN(Engine::FirstParty::MockupState)
SYNCFUNCTION(getLobbyListImpl)
SYNCFUNCTION(setLobbyPropertyImpl)
SYNCFUNCTION(leaveLobbyImpl)
SYNCFUNCTION(leaveMatchImpl)
SYNCFUNCTION(startMatchImpl)
SYNCFUNCTION(createLobbyImpl)
SYNCFUNCTION(unlockAchievementImpl)
SYNCFUNCTION(ingestStatImpl)
SYNCFUNCTION(joinLobbyImpl)
SYNCFUNCTION(updateLobbyInfoImpl)
SYNCFUNCTION(sendServerAddressImpl)
SERIALIZETABLE_END(Engine::FirstParty::MockupState)

namespace Engine {
namespace FirstParty {

    MockupState::MockupState()
    {
        Execution::sync_expect(mNetwork.addTopLevelItem(this, "State"));
    }

    void MockupState::update()
    {
        mNetwork.sendAndReceiveMessages();
    }

    std::vector<Lobby> MockupState::getLobbyListImpl()
    {
        throw 0;
    }

    SessionInfo MockupState::startMatchImpl(Serialize::SyncFunctionContext context)
    {
        throw 0;
    }

    std::optional<Lobby> MockupState::joinLobbyImpl(Serialize::SyncFunctionContext context, uint64_t lobbyId)
    {
        throw 0;
    }

    std::optional<Lobby> MockupState::createLobbyImpl(Serialize::SyncFunctionContext context, size_t maxPlayerCount, std::map<std::string, std::string> properties)
    {
        throw 0;
    }

    bool MockupState::unlockAchievementImpl(Serialize::SyncFunctionContext context, std::string_view name)
    {
        throw 0;
    }

    bool MockupState::ingestStatImpl(Serialize::SyncFunctionContext context, std::string_view name, std::string_view leaderboardName, int32_t value)
    {
        throw 0;
    }

    void MockupState::setLobbyPropertyImpl(Serialize::SyncFunctionContext context, std::string_view key, std::string_view value)
    {
        throw 0;
    }

    void MockupState::leaveLobbyImpl(Serialize::SyncFunctionContext context)
    {
        throw 0;
    }

    void MockupState::leaveMatchImpl(Serialize::SyncFunctionContext context)
    {
        throw 0;
    }

    void MockupState::updateLobbyInfoImpl(LobbyInfo info)
    {
        throw 0;
    }

    void MockupState::sendServerAddressImpl(SocketAddress address, SessionInfo session)
    {
        throw 0;
    }

    Threading::Task<std::vector<Lobby>> MockupState::getLobbyList()
    {
        co_return (co_await query<&MockupState::getLobbyListImpl>()).value();
    }

    Threading::Task<SessionInfo> MockupState::startMatch()
    {
        co_return (co_await query<&MockupState::startMatchImpl>()).value();
    }

    Threading::Task<std::optional<Lobby>> MockupState::joinLobby(uint64_t id)
    {
        co_return (co_await query<&MockupState::joinLobbyImpl>(id)).value();
    }

    Threading::Task<std::optional<Lobby>> MockupState::createLobby(size_t maxPlayerCount, std::map<std::string, std::string> properties)
    {
        co_return (co_await query<&MockupState::createLobbyImpl>(maxPlayerCount, properties)).value();
    }

    Threading::Task<bool> MockupState::unlockAchievement(std::string_view name)
    {
        co_return (co_await query<&MockupState::unlockAchievementImpl>(name)).value();
    }

    Threading::Task<bool> MockupState::ingestStat(std::string_view name, std::string_view leaderboardName, int32_t value)
    {
        co_return (co_await query<&MockupState::ingestStatImpl>(name, leaderboardName, value)).value();
    }

    void MockupState::setLobbyProperty(std::string_view key, std::string_view value)
    {
        command<&MockupState::setLobbyPropertyImpl>(key, value);
    }

    void MockupState::leaveLobby()
    {
        command<&MockupState::leaveLobbyImpl>();
    }

    void MockupState::leaveMatch()
    {
        command<&MockupState::leaveMatchImpl>();
    }

    void MockupState::updateLobbyInfo(const std::set<Serialize::ParticipantId> &targets, LobbyInfo info)
    {
        notify_some<&MockupState::updateLobbyInfoImpl>(targets, std::move(info));
    }

    void MockupState::sendServerAddress(const std::set<Serialize::ParticipantId> &targets, SocketAddress address, SessionInfo session)
    {
        notify_some<&MockupState::sendServerAddressImpl>(targets, std::move(address), std::move(session));
    }

}
}