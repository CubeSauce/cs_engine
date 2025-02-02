#include "cs/engine/net/socket.hpp"
#include "cs/engine/net/socket.hpp"

#include <cstdio>

Socket::Socket(Socket::Family fam, Socket::Type type, bool non_blocking)
{
#ifdef CS_PLATFORM_UNIX
    //file_descriptor = ::socket(fam, type | non_blocking ? SOCK_NONBLOCK : 0, 0);
    if (file_descriptor < 0)
    {
        //Error
        return;
    }
    
    server_addr.sin_family = static_cast<sa_family_t>(fam);
#elif defined CS_PLATFORM_WINDOWS
    int32 result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != NO_ERROR)
    {
        printf("WSAStartup failed with error: %d\n", result);
        return;
    }
    
    sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sock == INVALID_SOCKET)
    {
        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    if (non_blocking)
    {
        unsigned long mode;
        ioctlsocket(sock, FIONBIO, &mode);
    }

    server_addr.sin_family = static_cast<ADDRESS_FAMILY>(fam);
#endif
}

Socket::~Socket() 
{
#ifdef CS_PLATFORM_LINUX
#elif defined CS_PLATFORM_WINDOWS
    int32 result = closesocket(sock);
    if (result == SOCKET_ERROR)
    {
        wprintf(L"closesocket failed with error %d\n", WSAGetLastError());
        return;
    }
    WSACleanup();
#endif
}

//Todo: bind error check
void Socket::bind(unsigned port)
{
#ifdef CS_PLATFORM_LINUX
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    ::bind(file_descriptor, (sockaddr*)&server_addr, sizeof(server_addr));
#elif defined CS_PLATFORM_WINDOWS
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);		
    int32 result = ::bind(sock, (SOCKADDR*)&server_addr, sizeof(server_addr));
    if (result != 0)
    {
        wprintf(L"bind failed with error %d\n", WSAGetLastError());
        return;
    }
#endif
}

void Socket::set_port(uint16_t port)
{
    server_addr.sin_port = htons(port);
}

void Socket::set_address(const char* hostname)
{
#ifdef CS_PLATFORM_LINUX
    inet_aton(hostname, &(server_addr.sin_addr));
#elif defined CS_PLATFORM_WINDOWS
    inet_pton(AF_INET, hostname, &server_addr.sin_addr);
#endif
}
