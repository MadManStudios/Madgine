
#include "Madgine/mockupliblib.h"

#include "Madgine/serialize/network/networkmanager.h"

#include "Generic/execution/execution.h"

#include "mockupstate.h"

#include "Meta/serialize/container/noparent.h"

#include "Meta/serialize/formats.h"

using namespace Engine;

namespace Engine {
namespace FirstParty {

    struct ServerState : MockupState {

        void update()
        {
            mNetwork.acceptConnections(Serialize::Formats::xml);
            MockupState::update();
        }

        Network::NetworkManagerResult startServer(int port)
        {
            return mNetwork.startServer(port);
        }

        struct MockupLobby {
            operator LobbyInfo() const
            {
                auto members = mMembers | std::views::transform(LIFT(std::to_string));
                return {
                    { members.begin(), members.end() },
                    mProperties
                };
            }

            Serialize::ParticipantId mOwner;
            std::map<std::string, std::string> mProperties;
            std::set<Serialize::ParticipantId> mMembers;
        };

        auto findLobby(Serialize::ParticipantId userId)
        {
            return std::ranges::find_if(mLobbies, [=](const MockupLobby &lobby) { return std::ranges::contains(lobby.mMembers, userId); }, &std::pair<const uint64_t, MockupLobby>::second);
        }

        static Lobby toLobby(const std::pair<const uint64_t, MockupLobby> &p)
        {
            return { p.first, p.second.mProperties };
        }

        std::vector<Lobby> getLobbyListImpl() override
        {
            auto view = mLobbies | std::views::transform(&ServerState::toLobby);
            return { view.begin(), view.end() };
        }

        SessionInfo startMatchImpl(Serialize::SyncFunctionContext context) override
        {
            SessionInfo result;

            auto it = findLobby(context.mCallerId);
            std::ranges::transform(it->second.mMembers, std::back_inserter(result.mPlayers), [](Serialize::ParticipantId id) { return PlayerInfo { std::to_string(id) }; });
            
            it->second.mMembers.erase(context.mCallerId);
            sendServerAddress(it->second.mMembers, mNetwork.getAddress(context.mCallerId), result);
            
            mLobbies.erase(it);

            return result;
        }

        std::optional<Lobby> joinLobbyImpl(Serialize::SyncFunctionContext context, uint64_t lobbyId) override
        {
            auto it = mLobbies.find(lobbyId);
            it->second.mMembers.insert(context.mCallerId);
            updateLobbyInfo(it->second.mMembers, it->second);
            return toLobby(*it);
        }

        std::optional<Lobby> createLobbyImpl(Serialize::SyncFunctionContext context, size_t maxPlayerCount, std::map<std::string, std::string> properties) override
        {
            auto pib = mLobbies.try_emplace(mRunningId++, context.mCallerId, std::move(properties));
            assert(pib.second);
            MockupLobby &lobby = pib.first->second;
            lobby.mMembers.insert(context.mCallerId);
            updateLobbyInfo(lobby.mMembers, lobby);

            LOG("Created lobby " << pib.first->first << " for player " << context.mCallerId);
            LOG("Properties: ");
            for (const auto &[key, value] : lobby.mProperties)
                LOG("\t" << key << ": " << value);

            return toLobby(*pib.first);
        }

        bool unlockAchievementImpl(Serialize::SyncFunctionContext context, std::string_view name) override
        {
            LOG("Unlocking achievement \"" << name << " for user " << context.mCallerId);
            return true;
        }

        bool ingestStatImpl(Serialize::SyncFunctionContext context, std::string_view name, std::string_view leaderboardName, int32_t value) override
        {
            LOG("Setting stat \"" << name << "\" in leaderboard \"" << leaderboardName << "\" to " << value << " for user " << context.mCallerId);
            return true;
        }

        void setLobbyPropertyImpl(Serialize::SyncFunctionContext context, std::string_view key, std::string_view value) override
        {
            auto &[id, lobby] = *findLobby(context.mCallerId);
            lobby.mProperties[std::string { key }] = value;
            updateLobbyInfo(lobby.mMembers, lobby);
        }

        void leaveLobbyImpl(Serialize::SyncFunctionContext context) override
        {
            auto it = findLobby(context.mCallerId);
            if (it != mLobbies.end()) {
                auto &[id, lobby] = *it;
                lobby.mMembers.erase(context.mCallerId);
                updateLobbyInfo(lobby.mMembers, lobby);
            }
        }

        void leaveMatchImpl(Serialize::SyncFunctionContext context) override
        {
        }

        std::map<uint64_t, MockupLobby> mLobbies;

        uint64_t mRunningId = 1;

    };
}
}

int main()
{
    Serialize::NoParent<FirstParty::ServerState> state;

    Network::NetworkManagerResult result = state.startServer(12346);
    if (result != Network::NetworkManagerResult { Network::NetworkManagerResult::SUCCESS }) {
        LOG_ERROR("Failed to start Mockup server: " << result);
        return -1;
    }
    LOG("Mockup server started!");

    while (true) {
        state.update();
        std::this_thread::sleep_for(1s);
    }
}