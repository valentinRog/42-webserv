#include "ServerConf.hpp"

/* -------------------------------------------------------------------------- */

ServerConf::Location::Location() : autoindex( false ) {}

ServerConf::Location::Location( const JSON::Object &o ) : autoindex( false ) {
    if ( o.count( "autoindex" ) ) {
        autoindex = o.at( "autoindex" ).unwrap< JSON::Boolean >();
    }
    path = o.at( "path" ).unwrap< JSON::String >();
    if ( o.count( "root" ) ) { root = o.at( "root" ).unwrap< JSON::String >(); }
    if ( o.count( "methods" ) ) {
        JSON::Array a( o.at( "methods" ).unwrap< JSON::Array >() );
        for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
              it++ ) {
            methods.insert( it->unwrap< JSON::String >() );
        }
    }
}

/* -------------------------------------------------------------------------- */

ServerConf::ServerConf() {}

ServerConf::ServerConf( const JSON::Object &o ) : con_to( 0 ), idle_to( 0 ) {
    try {
        ::bzero( &addr, sizeof addr );
        addr.sin_family = AF_INET;
        addr.sin_port   = htons( o.at( "listen" )
                                   .unwrap< JSON::Object >()
                                   .at( "port" )
                                   .unwrap< JSON::Number >() );
        if ( o.at( "listen" ).unwrap< JSON::Object >().count( "address" ) ) {
            addr.sin_addr.s_addr
                = ::inet_addr( std::string( o.at( "listen" )
                                                .unwrap< JSON::Object >()
                                                .at( "address" )
                                                .unwrap< JSON::String >() )
                                   .c_str() );
        } else {
            addr.sin_addr.s_addr = htonl( INADDR_ANY );
        }
        if ( o.count( "server_names" ) ) {
            JSON::Array a( o.at( "server_names" ).unwrap< JSON::Array >() );
            for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
                  it++ ) {
                names.insert( it->unwrap< JSON::String >() );
            }
        }
        JSON::Array a( o.at( "location" ).unwrap< JSON::Array >() );
        for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
              it++ ) {
            Location location( it->unwrap< JSON::Object >() );
            locations.push_back( location );
        }
    } catch ( const std::out_of_range & ) {
        throw ConfigError();
    } catch ( const std::bad_cast & ) { throw ConfigError(); }
}

const char *ServerConf::ConfigError::what() const throw() {
    return "Invalid configuration";
}

/* -------------------------------------------------------------------------- */
