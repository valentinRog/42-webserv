#include "ServerConf.hpp"

ServerConf::ServerConf( uint16_t port ) {
    ::bzero( &_addr, sizeof _addr );
    _addr.sin_family = AF_INET;
    _addr.sin_port   = htons( port );
}

sockaddr_in &ServerConf::get_addr() { return _addr; }
