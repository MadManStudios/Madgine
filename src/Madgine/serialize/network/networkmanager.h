#pragma once

#include "Platform/socket/socketapi.h"

#include "Meta/serialize/syncmanager.h"

#undef SOCKET_ERROR

namespace Engine {
namespace Serialize {

    ENUM_BASE(NetworkManagerResult, SyncManagerResult,
        ALREADY_CONNECTED,
        NO_SERVER,
        SOCKET_ERROR)

    struct MADGINE_NETWORK_SERIALIZE_EXPORT NetworkManager : SyncManager {
        NetworkManager(const std::string &name);
        NetworkManager(const NetworkManager &) = delete;
        NetworkManager(NetworkManager &&) noexcept;
        virtual ~NetworkManager();

        void operator=(const NetworkManager &) = delete;

        NetworkManagerResult startServer(int port);
        Execution::Future<NetworkManagerResult> connect(std::string_view url, int portNr, Format format, TimeOut timeout = {});

        Platform::SocketAddress getAddress(Serialize::ParticipantId id);

        void close();

        NetworkManagerResult acceptConnection(Serialize::Format format, TimeOut timeout = {});
        int acceptConnections(Serialize::Format format, int limit = -1, TimeOut timeout = 0ms);

        bool isConnected() const;
        bool isServer() const;

        NetworkManagerResult moveMasterStream(Serialize::ParticipantId streamId,
            NetworkManager *target);

        Platform::SocketAPIResult getSocketAPIError() const;

    protected:
        NetworkManagerResult recordSocketError(Platform::SocketAPIResult error);

    private:
        Platform::Socket mServerSocket;

        Platform::SocketAPIResult mSocketAPIError = Platform::SocketAPIResult::SUCCESS;
    };
}
}
