
#include "cs/engine/net/net_connection.hpp"

Net_Connection::Net_Connection(Net_Type::Type in_net_role)
    : _net_role(in_net_role)
{
    _init_net_role();
}

void Net_Connection::update(float dt)
{
    switch(_net_role)
    {
    case Net_Type::Offline:
    {
        break;
    }
    case Net_Type::Listen_Server:
    {
        break;
    }
    case Net_Type::Server:
    {
        break;
    }
    case Net_Type::Client:
    {
        break;
    }
    }
}

void Net_Connection::_init_net_role()
{
    _socket = Shared_Ptr<Socket>::create(Socket::Family::IPV4, Socket::Type::DATAGRAM);

    switch(_net_role)
    {
    case Net_Type::Offline: _init_offline(); break;
    case Net_Type::Listen_Server: _init_listen_server(); break;
    case Net_Type::Server: _init_server(); break;
    case Net_Type::Client: _init_client(); break;
    default: assert(false);
    }
}

void Net_Connection::_init_offline()
{
}

void Net_Connection::_init_listen_server()
{
    _socket->bind(port);
}

void Net_Connection::_init_server()
{
    _socket->bind(port);
}

void Net_Connection::_init_client()
{
    _socket->set_address(server_address.c_str());
    _socket->set_port(port);
}
