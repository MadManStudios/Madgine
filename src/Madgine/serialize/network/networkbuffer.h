#pragma once

#include "Platform/socket/socketapi.h"

namespace Engine {
namespace Serialize {
    struct MADGINE_NETWORK_SERIALIZE_EXPORT NetworkBuffer : std::basic_streambuf<char> {
        NetworkBuffer(Platform::Socket socket);
        NetworkBuffer(const NetworkBuffer &) = delete;
        NetworkBuffer(NetworkBuffer &&other) noexcept = delete;
        virtual ~NetworkBuffer();

        Platform::SocketAddress getAddress() const;

    protected:
        // void handleError() override;

        std::streamsize xsgetn(char *, std::streamsize) override;

        std::streamsize xsputn(const char *, std::streamsize) override;

        std::streamsize showmanyc() override;

    private:
        Platform::Socket mSocket; // = SOCKET
    };
}
}
