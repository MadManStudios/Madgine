#pragma once

#include "Generic/timeout.h"

#include "Generic/genericresult.h"

namespace Engine {

using SocketAddress = std::string;

ENUM_BASE(SocketAPIResult, GenericResult,
    WOULD_BLOCK,
    TIMEOUT,
    CONNECTION_REFUSED,
    ALREADY_IN_USE,
    API_VERSION_MISMATCH)

struct INTERFACES_EXPORT Socket {
    Socket() = default;
    Socket(const Socket &) = delete;
    Socket(Socket &&other)
        : mSocket(std::exchange(other.mSocket, Invalid_Socket))
    {
    }

    ~Socket()
    {
        if (*this)
            close();
    }

    SocketAPIResult open(int port);
    SocketAPIResult connect(std::string_view url, int portNr);

    void close();

    SocketAddress address() const;

    int send(const char *buf, size_t len) const;
    int recv(char *buf, size_t len) const;

    SocketAPIResult accept(const Socket &from, TimeOut timeout = {});

    int in_available() const;

    Socket& operator=(Socket&& other) {
        assert(!(*this));
        mSocket = std::exchange(other.mSocket, Invalid_Socket);
        return *this;
    }

    explicit operator bool() const
    {
        return mSocket != Invalid_Socket;
    }

private:
    static const constexpr unsigned long long Invalid_Socket = -1;
    unsigned long long mSocket = Invalid_Socket;
};

namespace SocketAPI {
    INTERFACES_EXPORT SocketAPIResult init();
    INTERFACES_EXPORT void finalize();

    INTERFACES_EXPORT SocketAPIResult getError(const char *operation);
    INTERFACES_EXPORT int getOSError();
}

}
