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

namespace Net_Type
{
    enum Type : uint8
    {
        Offline,
        Listen_Server,
        Server,
        Client,
    };
}

class Net_Connection
{
public:
    // Only used for Net_Role::Client
    std::string server_address { "127.0.0.1" };
    uint16 port = 50005;

public:
    Net_Connection() = default;
    Net_Connection(Net_Type::Type in_net_role);

    void update(float dt);

    Net_Type::Type get_role() const { return _net_role; }
    bool is_local() const { return _net_role != Net_Type::Server; }

private:
    Net_Type::Type _net_role { Net_Type::Offline };
    Shared_Ptr<Socket> _socket;

private:
    void _init_net_role();
    void _init_offline();
    void _init_listen_server();
    void _init_server();
    void _init_client();
};
