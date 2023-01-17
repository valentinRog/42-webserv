#include "ServerConf.hpp"

ServerConf::ServerConf( const JSON::Object &conf_o )
    : _con_to( 0 ),
      _idle_to( 0 ) {
    ::bzero( &_addr, sizeof _addr );
    _addr.sin_family = AF_INET;
    JSON::Object o( conf_o.at( "listen" ).unwrap< JSON::Object >() );
    _addr.sin_port = htons( o.at( "port" ).unwrap< JSON::Number >() );
    if ( o.count( "address" ) ) {
        _addr.sin_addr.s_addr = ::inet_addr(
            std::string( o.at( "address" ).unwrap< JSON::String >() ).c_str() );
    } else {
        _addr.sin_addr.s_addr = htonl( INADDR_ANY );
    }
}

const sockaddr_in &ServerConf::get_addr() const { return _addr; }
time_t             ServerConf::get_con_to() const { return _con_to; }
time_t             ServerConf::get_idle_to() const { return _idle_to; }

const std::set< std::string > &ServerConf::get_names() const { return _names; }

const char *ServerConf::ConfigError::what() const throw() {
    return "Invalid configuration";
}
