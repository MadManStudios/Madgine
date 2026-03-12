#pragma once

#include "Interfaces/socket/socketapi.h"

#include "Meta/serialize/syncmanager.h"

#undef SOCKET_ERROR

namespace Engine {
namespace Network {

    ENUM_BASE(NetworkManagerResult, Serialize::SyncManagerResult,
        ALREADY_CONNECTED,
        NO_SERVER,
        SOCKET_ERROR)

    struct MADGINE_NETWORK_SERIALIZE_EXPORT NetworkManager : Serialize::SyncManager {
        NetworkManager(const std::string &name);
        NetworkManager(const NetworkManager &) = delete;
        NetworkManager(NetworkManager &&) noexcept;
        virtual ~NetworkManager();

        void operator=(const NetworkManager &) = delete;

        NetworkManagerResult startServer(int port);
        Execution::Future<NetworkManagerResult> connect(std::string_view url, int portNr, Serialize::Format format, TimeOut timeout = {});

        SocketAddress getAddress(Serialize::ParticipantId id);

        void close();

        NetworkManagerResult acceptConnection(Serialize::Format format, TimeOut timeout = {});
        int acceptConnections(Serialize::Format format, int limit = -1, TimeOut timeout = 0ms);

        bool isConnected() const;
        bool isServer() const;

        NetworkManagerResult moveMasterStream(Serialize::ParticipantId streamId,
            NetworkManager *target);

        SocketAPIResult getSocketAPIError() const;

    protected:
        NetworkManagerResult recordSocketError(SocketAPIResult error);

    private:
        Socket mServerSocket;

        SocketAPIResult mSocketAPIError = SocketAPIResult::SUCCESS;
    };
}
}
