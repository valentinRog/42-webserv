#include "ServerConf.hpp"

ServerConf::ServerConf( const JSON::Object &o ) {
    _addr.sin_family = AF_INET;
    _addr.sin_port   = htons(
        dynamic_cast< const JSON::Number * >( o.get().at( "listen" ) )->get() );
}

const sockaddr_in &ServerConf::get_addr() const { return _addr; }
