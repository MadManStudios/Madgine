#pragma once

#include <steam/steam_api.h>

#include "Meta/serialize/streams/message_streambuf.h"
#include "Generic/functor.h"

namespace Engine {
namespace FirstParty {

    struct SteamMessageHeader {
        Serialize::MessageId mMessageId;
    };

    struct SteamMessagePtr : std::unique_ptr<SteamNetworkingMessage_t, Functor<&SteamNetworkingMessage_t::Release>> {
        using std::unique_ptr<SteamNetworkingMessage_t, Functor<&SteamNetworkingMessage_t::Release>>::unique_ptr;
        SteamMessagePtr(SteamMessagePtr &&) = default;

        SteamMessagePtr &operator=(SteamMessagePtr &&) = default;

        SteamNetworkingMessage_t **operator&()
        {
            assert(!*this);
            return reinterpret_cast<SteamNetworkingMessage_t **>(this);
        }

        SteamNetworkingMessage_t *const *operator&() const
        {
            return reinterpret_cast<SteamNetworkingMessage_t *const *>(this);
        }
    };


    struct SteamStreambuf : Serialize::message_streambuf {

        SteamStreambuf(HSteamNetConnection con);
        ~SteamStreambuf();

    protected:
        void beginMessageWriteImpl() override;
        Serialize::MessageId endMessageWriteImpl() override;

        Serialize::MessageId beginMessageReadImpl() override;
        std::streamsize endMessageReadImpl() override;

        pos_type seekoff(off_type off, std::ios_base::seekdir dir,
            std::ios_base::openmode mode = std::ios_base::in) override;
        pos_type seekpos(pos_type pos,
            std::ios_base::openmode mode = std::ios_base::in) override;

        int_type overflow(int c = EOF) override;

        void extend();
        Serialize::StreamResult receiveMessages() override;
        Serialize::StreamResult sendMessages() override;

    private:
        HSteamNetConnection mConnection;

        std::unique_ptr<char[]> mSendBuffer;
        size_t mSendBufferSize = 0;

        std::vector<SteamMessagePtr> mSendMessages;

        SteamMessagePtr mReceivingMessage;

        Serialize::MessageId mRunningMessageId = 0;
    };

}
}