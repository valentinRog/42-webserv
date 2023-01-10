#include "ServerConf.hpp"

ServerConf::ServerConf( const JSON::Object &o ) {
    _addr.sin_family      = AF_INET;
    _addr.sin_port        = htons( o.at( "listen" ).unwrap< JSON::Number >() );
    _addr.sin_addr.s_addr = inet_addr(
        std::string( o.at( "server_name" ).unwrap< JSON::String >() ).c_str() );
    if ( o.count( "timeout" ) ) {
        JSON::Object to( o.at( "timeout" ).unwrap< JSON::Object >() );
        if ( o.count( "connection" ) ) {
            _con_to = o.at( "connection" ).unwrap< JSON::Number >();
        }
        if ( o.count( "idle" ) ) {
            _idle_to = o.at( "idle" ).unwrap< JSON::Number >();
        }
    }
}

const sockaddr_in &ServerConf::get_addr() const { return _addr; }

const char *ServerConf::ConfigError::what() const throw() {
    return "Invalid configuration";
}
