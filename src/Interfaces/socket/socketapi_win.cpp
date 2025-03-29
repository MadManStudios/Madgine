#include "../interfaceslib.h"

#if WINDOWS

#    include "socketapi.h"

#    include <WS2tcpip.h>
#    include <WinSock2.h>

#    undef NO_ERROR

namespace Engine {

SocketAPIResult toResult(int error, const char *op)
{
    switch (error) {
    case WSAECONNREFUSED:
        return SocketAPIResult::CONNECTION_REFUSED;
    case WSAEWOULDBLOCK:
        return SocketAPIResult::WOULD_BLOCK;
    default:
        fprintf(stderr, "Unknown Windows Socket-Error-Code from %s: %d\n", op, error);
        fflush(stderr);
        return SocketAPIResult::UNKNOWN_ERROR;
    }
}

SocketAPIResult SocketAPI::init()
{
    WSADATA w;

    int error = WSAStartup(MAKEWORD(2, 2), &w);

    if (error) {
        return toResult(error, "WSAStartup");
    }

    if (w.wVersion != MAKEWORD(2, 2)) {
        WSACleanup();
        return SocketAPIResult::API_VERSION_MISMATCH;
    }
    return SocketAPIResult::SUCCESS;
}

void SocketAPI::finalize()
{
    WSACleanup();
}

void Socket::close()
{
    assert(mSocket);
    int result = closesocket(mSocket);
    assert(result == 0);
    mSocket = Invalid_Socket;
}

SocketAddress Socket::address() const {
    sockaddr address;    
    char ext[16];
    int length = sizeof(sockaddr) + sizeof(ext);
    if (getpeername(mSocket, &address, &length))
        return {};

    char buffer[256];
    switch (address.sa_family) {
    case AF_INET:
        inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in&>(address).sin_addr,
            buffer, 256);
        break;

    case AF_INET6:
        inet_ntop(AF_INET6, &reinterpret_cast<sockaddr_in6&>(address).sin6_addr,
            buffer, 256);
        break;

    default:
        return {};
    }
    return buffer;
}

int Socket::send(const char *buf, size_t len) const
{
    return ::send(mSocket, buf, static_cast<int>(len), 0);
}

int Socket::recv(char *buf, size_t len) const
{
    return ::recv(mSocket, buf, static_cast<int>(len), 0);
}

int Socket::in_available() const
{
    u_long bytes_available;
    if (ioctlsocket(mSocket, FIONREAD, &bytes_available) == SOCKET_ERROR)
        return -1;
    return bytes_available;
}

SocketAPIResult SocketAPI::getError(const char *op)
{
    return toResult(WSAGetLastError(), op);
}

int SocketAPI::getOSError()
{
    return WSAGetLastError();
}

SocketAPIResult Socket::open(int port)
{
    if (*this)
        return SocketAPIResult::ALREADY_IN_USE;

    SOCKADDR_IN addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s == INVALID_SOCKET) {
        return SocketAPI::getError("socket");
    }

    mSocket = s;
    
    if (bind(mSocket, LPSOCKADDR(&addr), sizeof addr) == SOCKET_ERROR) {
        SocketAPIResult result = SocketAPI::getError("bind");
        close();
        return result;
    }

    int result = listen(s, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        SocketAPIResult result = SocketAPI::getError("listen");
        close();
        return result;
    }
    return SocketAPIResult::SUCCESS;
}

SocketAPIResult Socket::accept(const Socket &from, TimeOut timeout)
{
    if (*this)
        return SocketAPIResult::ALREADY_IN_USE;

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(from.mSocket, &readSet);
    timeval timeout_s;
    timeval *timeout_p = &timeout_s;
    if (timeout.isInfinite()) {
        timeout_p = nullptr;
    }else if (timeout.isZero()) {
        timeout_s.tv_sec = 0;
        timeout_s.tv_usec = 0;
    } else {
        std::chrono::milliseconds remainder = std::chrono::duration_cast<std::chrono::milliseconds>(timeout.remainder());
        timeout_s.tv_sec = static_cast<long>(remainder.count()) / 1000;
        timeout_s.tv_usec = static_cast<long>(remainder.count()) % 1000 * 1000;
    }
    if (int error = ::select(static_cast<int>(from.mSocket), &readSet, nullptr, nullptr, timeout_p); error <= 0) {
        if (error == 0)
            return SocketAPIResult::TIMEOUT;
        else
            return SocketAPI::getError("select");
    }

    mSocket = ::accept(from.mSocket, nullptr, nullptr);
    assert(mSocket != INVALID_SOCKET);

    u_long iMode = 1;
    if (ioctlsocket(mSocket, FIONBIO, &iMode) == SOCKET_ERROR) {
        SocketAPIResult result = SocketAPI::getError("accept");
        close();
        return result;
    }
    return SocketAPIResult::SUCCESS;
}

SocketAPIResult Socket::connect(std::string_view url, int portNr)
{
    if (*this)
        return SocketAPIResult::ALREADY_IN_USE;

    //Fill out the information needed to initialize a socket…
    SOCKADDR_IN target; //Socket address information

    target.sin_family = AF_INET; // address family Internet
    target.sin_port = htons(portNr); //Port to connect on
    InetPton(AF_INET, url.data(), &target.sin_addr.s_addr);

    SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create socket

    if (s == INVALID_SOCKET) {
        return SocketAPI::getError("socket");
    }

    mSocket = s;

    //Try connecting...

    if (::connect(s, reinterpret_cast<SOCKADDR *>(&target), sizeof target) == SOCKET_ERROR) {
        SocketAPIResult error = SocketAPI::getError("connect");
        close();
        return error;
    }

    u_long iMode = 1;
    if (ioctlsocket(s, FIONBIO, &iMode)) {
        SocketAPIResult error = SocketAPI::getError("ioctlsocket");
        close();
        return error;
    }

    return SocketAPIResult::SUCCESS;
}
}

#endif
