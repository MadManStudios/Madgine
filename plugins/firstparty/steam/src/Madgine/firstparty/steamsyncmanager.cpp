#include "../steamserviceslib.h"
#include "steamsyncmanager.h"

#include "steamsyncmanager.h"

#include <steam/steam_api.h>

#include "steamstreambuf.h"

#include "steamstreamdata.h"

#include "Meta/serialize/streams/syncstreamdata.h"

namespace Engine {
namespace FirstParty {

    void SteamSyncManager::setup()
    {
        SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, [](ESteamNetworkingSocketsDebugOutputType nType, const char *pszMsg) {
            Log::MessageType type;
            switch (nType) {
            case k_ESteamNetworkingSocketsDebugOutputType_Bug:
                type = Log::MessageType::FATAL_TYPE;
                break;
            case k_ESteamNetworkingSocketsDebugOutputType_Error:
                type = Log::MessageType::ERROR_TYPE;
                break;
            case k_ESteamNetworkingSocketsDebugOutputType_Important:
            case k_ESteamNetworkingSocketsDebugOutputType_Warning:
                type = Log::MessageType::WARNING_TYPE;
                break;
            case k_ESteamNetworkingSocketsDebugOutputType_Msg:
                type = Log::MessageType::INFO_TYPE;
                break;
            default:
                throw 0;
            }
            Engine::Log::LogDummy out { type };
            out << pszMsg;
        });

        SteamNetworkingUtils()->InitRelayNetworkAccess();
    }

    void SteamSyncManager::listen(size_t count, Serialize::Format format)
    {
        mFormat = format;

        mRemainingPlayersToConnect = count;

        mListenSocket = SteamNetworkingSockets()->CreateListenSocketP2P(0, 0, nullptr);
    }

    void SteamSyncManager::connectImpl(Execution::VirtualReceiverBase<Serialize::SyncManagerResult> &receiver, CSteamID target, Serialize::Format format, TimeOut timeout)
    {
        SteamNetworkingIdentity identity;
        identity.SetSteamID(target);
        HSteamNetConnection con = SteamNetworkingSockets()->ConnectP2P(identity, 0, 0, nullptr);

        setSlaveStreamImpl(receiver, format, std::make_unique<SteamStreambuf>(con), timeout, std::make_unique<SteamStreamData>(*this, createStreamId(), target));
    }

    void SteamSyncManager::disconnect()
    {
        if (mListenSocket != k_HSteamListenSocket_Invalid) {
            SteamNetworkingSockets()->CloseListenSocket(mListenSocket);
            mListenSocket = k_HSteamListenSocket_Invalid;
            mStoredPlayers.clear();
        }
        removeAllStreams();
        clearTopLevelItems();
    }

    Execution::SignalStub<> &SteamSyncManager::playersConnected()
    {
        return mPlayersConnected;
    }

    Serialize::ParticipantId SteamSyncManager::resolvePlayerId(CSteamID id)
    {
        if (id == SteamUser()->GetSteamID())
            return Serialize::sLocalMasterParticipantId;

        for (const Serialize::FormattedMessageStream& out : getMasterMessageTargets()) {
            if (static_cast<SteamStreamData*>(out.data())->user() == id) {
                return out.id();
            }
        }
         
        throw 0;
    }

    void SteamSyncManager::removeSlaveStream(Serialize::SyncManagerResult reason)
    {
        SyncManager::removeSlaveStream(reason);
    }

    std::map<Serialize::ParticipantId, Serialize::FormattedMessageStream>::iterator SteamSyncManager::removeMasterStream(std::map<Serialize::ParticipantId, Serialize::FormattedMessageStream>::iterator it, Serialize::SyncManagerResult reason)
    {
        SteamStreamData *data = static_cast<SteamStreamData*>(it->second.data());

        mStoredPlayers[data->user()] = data->id();

        return SyncManager::removeMasterStream(it, reason);
    }

    void SteamSyncManager::onConnectionUpdate(SteamNetConnectionStatusChangedCallback_t *con)
    {
        if (con->m_info.m_hListenSocket != k_HSteamListenSocket_Invalid) {
            switch (con->m_info.m_eState) {
            case k_ESteamNetworkingConnectionState_Connecting: {
                EResult result = SteamNetworkingSockets()->AcceptConnection(con->m_hConn);
                assert(result == k_EResultOK);

                CSteamID user = con->m_info.m_identityRemote.GetSteamID();

                Serialize::ParticipantId id;
                auto it = mStoredPlayers.find(user);
                if (it != mStoredPlayers.end())
                    id = it->second;
                else
                    id = createStreamId();

                std::unique_ptr<SteamStreamData> data = std::make_unique<SteamStreamData>(*this, id, user);

                Serialize::SyncManagerResult result2 = addMasterStream(mFormat, std::make_unique<SteamStreambuf>(con->m_hConn), std::move(data));
                assert(result2 == Serialize::SyncManagerResult::SUCCESS);
                break;
            }
            case k_ESteamNetworkingConnectionState_Connected:
                if (--mRemainingPlayersToConnect == 0)
                    mPlayersConnected.emit();
                break;
            }
        }
    }

}
}