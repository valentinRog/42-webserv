#include "ServerConf.hpp"

/* -------------------------------------------------------------------------- */

ServerConf::Route::Route( const JSON::Object &o ) : _autoindex( false ) {
    _root = o.at( "root" ).unwrap< JSON::String >();
    if ( o.count( "index" ) ) {
        JSON::Array a( o.at( "index" ).unwrap< JSON::Array >() );
        for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
              it++ ) {
            _index.push_back( it->unwrap< JSON::String >() );
        }
    }
    if ( o.count( "autoindex" ) ) {
        _autoindex = o.at( "autoindex" ).unwrap< JSON::Boolean >();
    }
    if ( o.count( "methods" ) ) {
        JSON::Array a( o.at( "methods" ).unwrap< JSON::Array >() );
        for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
              it++ ) {
            _methods.insert( it->unwrap< JSON::String >() );
        }
    }
    if ( o.count( "redir" ) ) {
        _redir = o.at( "redir" ).unwrap< JSON::String >();
    }
}

const std::string &ServerConf::Route::root() const { return _root; }

const std::list< std::string > &ServerConf::Route::index() const {
    return _index;
}

const std::set< std::string > &ServerConf::Route::methods() const {
    return _methods;
}

bool ServerConf::Route::autoindex() const { return _autoindex; }

const std::string &ServerConf::Route::redir() const { return _redir; }

/* -------------------------------------------------------------------------- */

void ServerConf::RouteMapper::insert(
    const std::pair< std::string, Route > &v ) {
    std::string key( v.first );
    _routes.insert( key );
    _routes_table.insert( v );
}
const ServerConf::Route &
ServerConf::RouteMapper::at( const std::string &s ) const {
    return _routes_table.at( _routes.lower_bound( s ) );
}

std::size_t ServerConf::RouteMapper::count( const std::string &s ) const {
    return _routes_table.count( _routes.lower_bound( s ) );
}

std::string ServerConf::RouteMapper::suffix( const std::string &s ) const {
    std::string::size_type l( _routes.lower_bound( s ).size() );
    return s.substr( l, s.size() - l );
}
/* -------------------------------------------------------------------------- */

const char *ServerConf::ConfigError::what() const throw() {
    return "Invalid configuration";
}

/* -------------------------------------------------------------------------- */

ServerConf::ServerConf(
    const JSON::Object &                                o,
    Ptr::shared< std::map< std::string, std::string > > mime )
    : _mime( mime ) {
    try {
        ::bzero( &_addr, sizeof _addr );
        _addr.sin_family = AF_INET;
        JSON::Array a( o.at( "listen" ).unwrap< JSON::Array >() );
        _addr.sin_port = htons( a[1].unwrap< JSON::Number >() );
        if ( ( _addr.sin_addr.s_addr = ::inet_addr(
                   std::string( a[0].unwrap< JSON::String >() ).c_str() ) )
             == INADDR_NONE ) {
            throw std::runtime_error( "inet_addr" );
        }
        if ( o.count( "server_names" ) ) {
            JSON::Array a( o.at( "server_names" ).unwrap< JSON::Array >() );
            for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
                  it++ ) {
                _names.insert( it->unwrap< JSON::String >() );
            }
        }
        JSON::Object routes_o( o.at( "routes" ).unwrap< JSON::Object >() );
        for ( JSON::Object::const_iterator it( routes_o.begin() );
              it != routes_o.end();
              it++ ) {
            _route_mapper.insert(
                std::make_pair( it->first,
                                it->second.unwrap< JSON::Object >() ) );
        }
    } catch ( const std::out_of_range & ) {
        throw ConfigError();
    } catch ( const std::bad_cast & ) { throw ConfigError(); }
}

/* -------------------------------------------------------------------------- */

const sockaddr_in &ServerConf::addr() const { return _addr; }

const std::set< std::string > &ServerConf::names() const { return _names; }

const ServerConf::RouteMapper &ServerConf::route_mapper() const {
    return _route_mapper;
}

const std::map< std::string, std::string > &ServerConf::mime() const {
    return *_mime;
}

/* -------------------------------------------------------------------------- */
