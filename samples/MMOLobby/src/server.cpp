
#include "mmolobbylib.h"

#include "server.h"

#include "Modules/threading/awaitables/awaitabletimepoint.h"

namespace MMOLobby {

Server::Server()
    : ServerBase("MMOLobby-Server")
    , mNetwork("MMOLobby-Server")
{
    LOG("MMOLobby Main-Server started!");

    if (Engine::Network::NetworkManagerResult result = mNetwork.startServer(12345)) {
        LOG_ERROR(result);
        shutdown();
    }

    mTaskQueue.queue([this]() -> Engine::Threading::Task<void> {
        while (mTaskQueue.running()) {
            checkConnections();
            co_await 400ms;
        }
    });
}

Server::~Server()
{
    LOG("MMOLobby Main-Server shutdown!");
}

void Server::moveConnection(Engine::Serialize::ParticipantId id, Engine::Network::NetworkManager *to)
{
    mNetwork.moveMasterStream(id, to);
}

Engine::Network::NetworkManager &Server::network()
{
    return mNetwork;
}

void Server::checkConnections()
{
    mNetwork.acceptConnections(Engine::Serialize::Formats::xml);
    if (Engine::Serialize::StreamResult result = mNetwork.sendAndReceiveMessages(); result.mState != Engine::Serialize::StreamState::OK) {
        LOG_ERROR(result);
    }
}

void Server::performCommand(const std::string &cmd)
{
    {
        ServerBase::performCommand(cmd);
    }
}

}
