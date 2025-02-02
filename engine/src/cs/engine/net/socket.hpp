#pragma once

#include "cs/cs.hpp"

#include <cstdint>
#include <sys/types.h>
#ifdef CS_PLATFORM_UNIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#elif defined CS_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
#endif

struct Socket
{
    enum Family : uint8
    {
        IPV4 = AF_INET,
        IPV6 = AF_INET6
    };

    enum Type : uint8
    {
        STREAM = SOCK_STREAM,
        DATAGRAM = SOCK_DGRAM,
        SEQUENCE = SOCK_SEQPACKET,
        RAW = SOCK_RAW
    };

#ifdef CS_PLATFORM_UNIX
    int file_descriptor = -1;
#elif defined CS_PLATFORM_WINDOWS
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
#endif
    sockaddr_in server_addr;

    Socket(Socket::Family fam, Socket::Type type, bool non_blocking = true);
    ~Socket();

    void bind(unsigned port);
    void set_port(uint16 port);
    void set_address(const char* hostname);
    
    template <typename Type>
    void send(Type data) 
    {
#ifdef CS_PLATFORM_UNIX
        //sendto(file_descriptor, (void*)(&data), sizeof(Type), MSG_CONFIRM, (sockaddr*)&server_addr, sizeof(server_addr));
#elif defined CS_PLATFORM_WINDOWS
        int32 result = sendto(sock, (char*)(void*)&data, sizeof(data), 0, (SOCKADDR*)&server_addr, sizeof(server_addr));
        if (result == SOCKET_ERROR)
        {
            wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
            closesocket(sock);
            WSACleanup();
            return;
        }
#endif
    }

    template <typename Type>
    int recieve(sockaddr_in* cliaddr, Type *data)
    {
#ifdef CS_PLATFORM_UNIX
        socklen_t clen = sizeof(*cliaddr);
        return recvfrom(file_descriptor, data, sizeof(Type), MSG_WAITALL, (sockaddr*)cliaddr, &clen);
#elif defined CS_PLATFORM_WINDOWS
        int clen = sizeof(*cliaddr);
        int result = recvfrom(sock, (char*)(void*)data, sizeof(*data), 0, (SOCKADDR*)cliaddr, &clen);
        if (result == SOCKET_ERROR)
        {
            wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
        }

        return result;
#endif
    }
};
