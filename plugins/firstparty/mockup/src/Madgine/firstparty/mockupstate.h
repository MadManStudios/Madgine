#pragma once

#include "Meta/serialize/hierarchy/toplevelunit.h"

#include "Madgine/firstparty/firstpartyservices.h"

#include "Madgine/serialize/network/networkmanager.h"

namespace Engine {
namespace FirstParty {

    struct MADGINE_MOCKUP_LIB_EXPORT MockupState : Serialize::TopLevelUnit<MockupState> {
        MockupState();

        void update();

        Threading::Task<std::vector<Lobby>> getLobbyList();
        Threading::Task<SessionInfo> startMatch();
        Threading::Task<std::optional<Lobby>> joinLobby(uint64_t lobbyId);
        Threading::Task<std::optional<Lobby>> createLobby(size_t maxPlayerCount, std::map<std::string, std::string> properties);
        Threading::Task<bool> unlockAchievement(std::string_view name);
        Threading::Task<bool> ingestStat(std::string_view name, std::string_view leaderboardName, int32_t value);

        void setLobbyProperty(std::string_view key, std::string_view value);
        void leaveLobby();
        void leaveMatch();
        void updateLobbyInfo(const std::set<Serialize::ParticipantId> &targets, LobbyInfo info);
        void sendServerAddress(const std::set<Serialize::ParticipantId> &targets, SocketAddress address, SessionInfo session);

        virtual std::vector<Lobby> getLobbyListImpl();
        virtual SessionInfo startMatchImpl(Serialize::SyncFunctionContext context);
        virtual std::optional<Lobby> joinLobbyImpl(Serialize::SyncFunctionContext context, uint64_t lobbyId);
        virtual std::optional<Lobby> createLobbyImpl(Serialize::SyncFunctionContext context, size_t maxPlayerCount, std::map<std::string, std::string> properties);
        virtual bool unlockAchievementImpl(Serialize::SyncFunctionContext context, std::string_view name);
        virtual bool ingestStatImpl(Serialize::SyncFunctionContext context, std::string_view name, std::string_view leaderboardName, int32_t value);
        virtual void setLobbyPropertyImpl(Serialize::SyncFunctionContext context, std::string_view key, std::string_view value);
        virtual void leaveLobbyImpl(Serialize::SyncFunctionContext context);
        virtual void leaveMatchImpl(Serialize::SyncFunctionContext context);
        virtual void updateLobbyInfoImpl(LobbyInfo info);
        virtual void sendServerAddressImpl(SocketAddress address, SessionInfo session);

        
        Network::NetworkManager mNetwork { "MockupNetwork" };
    };

}
}