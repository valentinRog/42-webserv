#include "ServerConf.hpp"

ServerConf::ServerConf( const JSON::Object &o ) : _con_to( 0 ), _idle_to( 0 ) {
    ::bzero( &_addr, sizeof _addr );
    _addr.sin_family      = AF_INET;
    _addr.sin_port        = htons( o.at( "listen" ).unwrap< JSON::Number >() );
    _addr.sin_addr.s_addr = inet_addr(
        std::string( o.at( "server_name" ).unwrap< JSON::String >() ).c_str() );
    if ( o.count( "timeout" ) ) {
        JSON::Object to( o.at( "timeout" ).unwrap< JSON::Object >() );
        if ( to.count( "connection" ) ) {
            _con_to = to.at( "connection" ).unwrap< JSON::Number >();
        }
        if ( to.count( "idle" ) ) {
            _idle_to = to.at( "idle" ).unwrap< JSON::Number >();
        }
    }
}

const sockaddr_in &ServerConf::get_addr() const { return _addr; }
time_t             ServerConf::get_con_to() const { return _con_to; }
time_t             ServerConf::get_idle_to() const { return _idle_to; }

const char *ServerConf::ConfigError::what() const throw() {
    return "Invalid configuration";
}
