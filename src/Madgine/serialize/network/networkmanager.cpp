#include "networklib.h"

#include "networkmanager.h"

#include "networkbuffer.h"

#include "Meta/serialize/streams/formattedbufferedstream.h"

#include "Meta/serialize/streams/syncstreamdata.h"

#include "Meta/serialize/streams/buffered_streambuf.h"

namespace Engine {
namespace Network {
    static int sManagerCount = 0;

    NetworkManager::NetworkManager(const std::string &name)
        : SyncManager(name)
    {
        if (sManagerCount == 0) {
            if (SocketAPI::init() != SocketAPIResult::SUCCESS)
                std::terminate();
        }
        ++sManagerCount;
    }

    NetworkManager::NetworkManager(NetworkManager &&other) noexcept
        : SyncManager(std::forward<NetworkManager>(other))
        , mServerSocket(std::move(other.mServerSocket))
    {
        ++sManagerCount;
    }

    NetworkManager::~NetworkManager()
    {
        close();
        --sManagerCount;
        if (sManagerCount == 0) {
            SocketAPI::finalize();
        }
    }

    NetworkManagerResult NetworkManager::startServer(int port)
    {
        if (isServer())
            return NetworkManagerResult::ALREADY_CONNECTED;

        SocketAPIResult error = mServerSocket.open(port);

        if (!isServer()) {
            NetworkManagerResult result = recordSocketError(error);
            return result;
        }

        return NetworkManagerResult::SUCCESS;
    }

    void NetworkManager::connectImpl(Execution::VirtualReceiverBase<type_pack<NetworkManagerResult, Serialize::SyncManagerResult>> &receiver, std::string_view url, int portNr, Serialize::Format format, TimeOut timeout)
    {
        if (isConnected()) {
            receiver.set_error(NetworkManagerResult::ALREADY_CONNECTED);
            return;
        }

        Socket socket;
        SocketAPIResult error = socket.connect(url, portNr);

        if (!socket) {
            NetworkManagerResult result = recordSocketError(error);
            receiver.set_error(result);
            return;
        }

        setSlaveStreamImpl(receiver, format, std::make_unique<Engine::Serialize::buffered_streambuf>(std::make_unique<NetworkBuffer>(std::move(socket))), timeout);        
    }

    void NetworkManager::close()
    {
        if (isConnected() || isServer()) {
            removeAllStreams();
            if (mServerSocket) {
                mServerSocket.close();
            }
        }
    }

    int NetworkManager::acceptConnections(Serialize::Format format, int limit, TimeOut timeout)
    {
        int count = 0;
        if (isServer()) {
            Socket sock;
            SocketAPIResult error = sock.accept(mServerSocket, timeout);
            while (error != SocketAPIResult::TIMEOUT && (limit == -1 || count < limit)) {
                if (sock) {
                    if (addMasterStream(format, std::make_unique<Engine::Serialize::buffered_streambuf>(std::make_unique<NetworkBuffer>(std::move(sock)))) == Serialize::SyncManagerResult::SUCCESS) {
                        ++count;
                    }
                }
                error = sock.accept(mServerSocket, timeout);
            }
        }
        return count;
    }

    NetworkManagerResult NetworkManager::acceptConnection(Serialize::Format format, TimeOut timeout)
    {
        if (!isServer())
            return NetworkManagerResult::NO_SERVER;

        Socket sock;
        SocketAPIResult error = sock.accept(mServerSocket, timeout);
        if (!sock)
            return recordSocketError(error);
        
        return addMasterStream(format, std::make_unique<Engine::Serialize::buffered_streambuf>(std::make_unique<NetworkBuffer>(std::move(sock))));
    }

    bool NetworkManager::isConnected() const
    {
        return !isMaster() && !mReceivingMasterState;
    }

    bool NetworkManager::isServer() const
    {
        return static_cast<bool>(mServerSocket);
    }

    NetworkManagerResult NetworkManager::moveMasterStream(
        Serialize::ParticipantId streamId, NetworkManager *target)
    {
        return SyncManager::moveMasterStream(streamId, target);
    }

    SocketAPIResult NetworkManager::getSocketAPIError() const
    {
        return mSocketAPIError;
    }

    NetworkManagerResult NetworkManager::recordSocketError(SocketAPIResult error)
    {
        mSocketAPIError = error;
        return NetworkManagerResult::SOCKET_ERROR;
    }

}
}
