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
        Threading::Task<std::vector<PlayerInfo>> startMatch();
        Threading::Task<std::optional<Lobby>> joinLobby(uint64_t lobbyId);
        Threading::Task<std::optional<Lobby>> createLobby(std::map<std::string, std::string> properties);
        Threading::Task<bool> unlockAchievement(std::string_view name);
        Threading::Task<bool> ingestStat(std::string_view name, std::string_view leaderboardName, int32_t value);

        void setLobbyProperty(std::string_view key, std::string_view value);
        void leaveLobby();
        void leaveMatch();
        void updateLobbyInfo(const std::set<Serialize::ParticipantId> &targets, LobbyInfo info);
        void sendServerAddress(const std::set<Serialize::ParticipantId> &targets, SocketAddress address, std::vector<PlayerInfo> players);

        virtual std::vector<Lobby> getLobbyListImpl();
        virtual std::vector<PlayerInfo> startMatchImpl(Serialize::SyncFunctionContext context);
        virtual std::optional<Lobby> joinLobbyImpl(Serialize::SyncFunctionContext context, uint64_t lobbyId);
        virtual std::optional<Lobby> createLobbyImpl(Serialize::SyncFunctionContext context, std::map<std::string, std::string> properties);
        virtual bool unlockAchievementImpl(Serialize::SyncFunctionContext context, std::string_view name);
        virtual bool ingestStatImpl(Serialize::SyncFunctionContext context, std::string_view name, std::string_view leaderboardName, int32_t value);
        virtual void setLobbyPropertyImpl(Serialize::SyncFunctionContext context, std::string_view key, std::string_view value);
        virtual void leaveLobbyImpl(Serialize::SyncFunctionContext context);
        virtual void leaveMatchImpl(Serialize::SyncFunctionContext context);
        virtual void updateLobbyInfoImpl(LobbyInfo info);
        virtual void sendServerAddressImpl(SocketAddress address, std::vector<PlayerInfo> players);

        
        Network::NetworkManager mNetwork { "MockupNetwork" };
    };

}
}