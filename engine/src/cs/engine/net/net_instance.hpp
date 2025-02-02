#pragma once

#include "cs/cs.hpp"
#include "cs/engine/event.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/engine/net/socket.hpp"
#include "cs/containers/dynamic_array.hpp"

#include <string>

struct Net_Packet
{
    enum Type : uint8
    {
        None,
        Connection_Req,
        Connection_Ack,
        State_Update,
        Input_Update
    };

    Type type { None };
    uint32 size { 0 };
    Dynamic_Array<uint8> data;
};

namespace Net_Role
{
    enum Type : uint8
    {
        Offline,
        Listen_Server,
        Server,
        Client,
    };
}

class Net_Instance
{
public:
    // Only used for Net_Role::Client
    std::string server_address { "127.0.0.1" };
    uint16 port = 50005;

public:
    Net_Instance() = default;
    Net_Instance(Net_Role::Type in_net_role);

    void update(float dt);

    Net_Role::Type get_role() const { return _net_role; }
    bool is_local() const { return _net_role != Net_Role::Server; }

private:
    Net_Role::Type _net_role { Net_Role::Offline };
    Shared_Ptr<Socket> _socket;

private:
    void _init_net_role();
    void _init_offline();
    void _init_listen_server();
    void _init_server();
    void _init_client();
};
