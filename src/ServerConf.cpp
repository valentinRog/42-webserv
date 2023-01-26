#include "ServerConf.hpp"

/* -------------------------------------------------------------------------- */

ServerConf::Route::Route( const JSON::Object &o ) : autoindex( false ) {
    root = o.at( "root" ).unwrap< JSON::String >();
    if ( o.count( "index" ) ) {
        JSON::Array a( o.at( "index" ).unwrap< JSON::Array >() );
        for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
              it++ ) {
            index.push_back( it->unwrap< JSON::String >() );
        }
    }
    if ( o.count( "autoindex" ) ) {
        autoindex = o.at( "autoindex" ).unwrap< JSON::Boolean >();
    }
    if ( o.count( "methods" ) ) {
        JSON::Array a( o.at( "methods" ).unwrap< JSON::Array >() );
        for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
              it++ ) {
            methods.insert( it->unwrap< JSON::String >() );
        }
    }
    if (o.count("redir")) {
        redir = o.at("redir").unwrap<JSON::String>();

    }
}

/* -------------------------------------------------------------------------- */

ServerConf::ServerConf( const JSON::Object &o ) {
    try {
        ::bzero( &addr, sizeof addr );
        addr.sin_family = AF_INET;
        JSON::Array a( o.at( "listen" ).unwrap< JSON::Array >() );
        addr.sin_port = htons( a[1].unwrap< JSON::Number >() );
        if ( ( addr.sin_addr.s_addr = ::inet_addr(
                   std::string( a[0].unwrap< JSON::String >() ).c_str() ) )
             == INADDR_NONE ) {
            throw std::runtime_error( "inet_addr" );
        }
        if ( o.count( "server_names" ) ) {
            JSON::Array a( o.at( "server_names" ).unwrap< JSON::Array >() );
            for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
                  it++ ) {
                names.insert( it->unwrap< JSON::String >() );
            }
        }
        JSON::Object routes_o( o.at( "routes" ).unwrap< JSON::Object >() );
        for ( JSON::Object::const_iterator it( routes_o.begin() );
              it != routes_o.end();
              it++ ) {
            routes.insert( it->first );
            routes_table.insert(
                std::make_pair( it->first,
                                it->second.unwrap< JSON::Object >() ) );
        }
    } catch ( const std::out_of_range & ) {
        throw ConfigError();
    } catch ( const std::bad_cast & ) { throw ConfigError(); }
}

const char *ServerConf::ConfigError::what() const throw() {
    return "Invalid configuration";
}

/* -------------------------------------------------------------------------- */
