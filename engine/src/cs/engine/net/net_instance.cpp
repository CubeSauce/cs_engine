
#include "cs/engine/net/net_instance.hpp"
#include "cs/engine/game/game_instance.hpp"

Net_Instance::Net_Instance(Net_Role::Type in_net_role)
    : _net_role(in_net_role)
{
    _init_net_role();
}

void Net_Instance::update(float dt)
{
    switch(_net_role)
    {
    case Net_Role::Offline:
    {
        break;
    }
    case Net_Role::Listen_Server:
    {
        break;
    }
    case Net_Role::Server:
    {
        break;
    }
    case Net_Role::Client:
    {
        break;
    }
    }
}

void Net_Instance::_init_net_role()
{
    _socket = Shared_Ptr<Socket>::create(Socket::Family::IPV4, Socket::Type::DATAGRAM);

    switch(_net_role)
    {
    case Net_Role::Offline: _init_offline(); break;
    case Net_Role::Listen_Server: _init_listen_server(); break;
    case Net_Role::Server: _init_server(); break;
    case Net_Role::Client: _init_client(); break;
    default: assert(false);
    }
}

void Net_Instance::_init_offline()
{
}

void Net_Instance::_init_listen_server()
{
    _socket->bind(port);
}

void Net_Instance::_init_server()
{
    _socket->bind(port);
}

void Net_Instance::_init_client()
{
    _socket->set_address(server_address.c_str());
    _socket->set_port(port);
}
