#include "ServerConf.hpp"

/* ---------------------------- ServerConf::Route --------------------------- */

const BiMap< ServerConf::Route::e_config_key, std::string > &
ServerConf::Route::key_to_string() {
    struct f {
        static BiMap< e_config_key, std::string > init() {
            BiMap< e_config_key, std::string > m;
            m.insert( std::make_pair( ROOT, "root" ) );
            m.insert( std::make_pair( INDEX, "index" ) );
            m.insert( std::make_pair( METHODS, "methods" ) );
            m.insert( std::make_pair( CGI, "cgi" ) );
            m.insert( std::make_pair( AUTOINDEX, "autoindex" ) );
            m.insert( std::make_pair( REDIR, "redir" ) );
            return m;
        }
    };
    static const BiMap< e_config_key, std::string > m( f::init() );
    return m;
}

ServerConf::Route::Route( const JSON::Object &o ) : _autoindex( false ) {
    for ( JSON::Object::const_iterator it( o.begin() ); it != o.end(); it++ ) {
        key_to_string().at( it->first );
    }
    _root = o.at( key_to_string().at( ROOT ) ).unwrap< JSON::String >();
    if ( o.count( key_to_string().at( INDEX ) ) ) {
        JSON::Array a(
            o.at( key_to_string().at( INDEX ) ).unwrap< JSON::Array >() );
        for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
              it++ ) {
            _index.push_back( it->unwrap< JSON::String >() );
        }
    }
    if ( o.count( key_to_string().at( AUTOINDEX ) ) ) {
        _autoindex
            = o.at( key_to_string().at( AUTOINDEX ) ).unwrap< JSON::Boolean >();
    }
    if ( o.count( key_to_string().at( METHODS ) ) ) {
        JSON::Array a(
            o.at( key_to_string().at( METHODS ) ).unwrap< JSON::Array >() );
        for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
              it++ ) {
            _methods.insert( it->unwrap< JSON::String >() );
        }
    }
    if ( o.count( key_to_string().at( REDIR ) ) ) {
        _redir = o.at( key_to_string().at( REDIR ) ).unwrap< JSON::String >();
    }
    if ( o.count( key_to_string().at( CGI ) ) ) {
        JSON::Object cgi_o(
            o.at( key_to_string().at( CGI ) ).unwrap< JSON::Object >() );
        for ( JSON::Object::const_iterator it = cgi_o.begin();
              it != cgi_o.end();
              it++ ) {
            _cgis[it->first] = it->second.unwrap< JSON::String >();
        }
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

const std::map< std::string, std::string > &ServerConf::Route::cgis() const {
    return _cgis;
}

/* ------------------------- ServerConf::RouteMapper ------------------------ */

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

size_t ServerConf::RouteMapper::count( const std::string &s ) const {
    return _routes_table.count( _routes.lower_bound( s ) );
}

std::string ServerConf::RouteMapper::route_name( const std::string &s ) const {
    return _routes.lower_bound( s );
}

std::string ServerConf::RouteMapper::suffix( const std::string &s ) const {
    std::string::size_type l( _routes.lower_bound( s ).size() );
    return s.substr( l, s.size() - l );
}

/* ------------------ ServerConf::RouteMapper::ConfigError ------------------ */

const char *ServerConf::ConfigError::what() const throw() {
    return "Invalid configuration";
}

/* ------------------------------- ServerConf ------------------------------- */

const BiMap< ServerConf::e_config_key, std::string > &
ServerConf::key_to_string() {
    struct f {
        static BiMap< e_config_key, std::string > init() {
            BiMap< e_config_key, std::string > m;
            m.insert( std::make_pair( LISTEN, "listen" ) );
            m.insert( std::make_pair( SERVER_NAMES, "names" ) );
            m.insert( std::make_pair( CLIENT_MAX_BODY_SIZE,
                                      "client_max_body_size" ) );
            m.insert( std::make_pair( ROUTES, "routes" ) );
            m.insert( std::make_pair( ERROR_PAGES, "error_pages" ) );
            return m;
        }
    };
    static const BiMap< e_config_key, std::string > m( f::init() );
    return m;
}

ServerConf::ServerConf( const JSON::Object &o )
    : _client_max_body_size( std::numeric_limits< std::size_t >::max() ) {
    try {
        for ( JSON::Object::const_iterator it( o.begin() ); it != o.end();
              it++ ) {
            key_to_string().at( it->first );
        }
        ::bzero( &_addr, sizeof _addr );
        _addr.sin_family = AF_INET;
        JSON::Array a(
            o.at( key_to_string().at( LISTEN ) ).unwrap< JSON::Array >() );
        _addr.sin_port = htons( a[1].unwrap< JSON::Number >() );
        if ( ( _addr.sin_addr.s_addr = ::inet_addr(
                   std::string( a[0].unwrap< JSON::String >() ).c_str() ) )
             == INADDR_NONE ) {
            throw std::runtime_error( "inet_addr" );
        }
        if ( o.count( key_to_string().at( SERVER_NAMES ) ) ) {
            JSON::Array a( o.at( key_to_string().at( SERVER_NAMES ) )
                               .unwrap< JSON::Array >() );
            for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
                  it++ ) {
                _names.insert( it->unwrap< JSON::String >() );
            }
        }
        JSON::Object routes_o(
            o.at( key_to_string().at( ROUTES ) ).unwrap< JSON::Object >() );
        for ( JSON::Object::const_iterator it( routes_o.begin() );
              it != routes_o.end();
              it++ ) {
            _route_mapper.insert(
                std::make_pair( it->first,
                                it->second.unwrap< JSON::Object >() ) );
        }
        if ( o.count( key_to_string().at( CLIENT_MAX_BODY_SIZE ) ) ) {
            _client_max_body_size
                = o.at( key_to_string().at( CLIENT_MAX_BODY_SIZE ) )
                      .unwrap< JSON::Number >();
        }
        if ( o.count( key_to_string().at( ERROR_PAGES ) ) ) {
            JSON::Object error_pages_o(
                o.at( key_to_string().at( ERROR_PAGES ) )
                    .unwrap< JSON::Object >() );
            for ( JSON::Object::const_iterator it( error_pages_o.begin() );
                  it != error_pages_o.end();
                  it++ ) {
                JSON::Array a( it->second.unwrap< JSON::Array >() );
                for ( JSON::Array::const_iterator it2( a.begin() );
                      it2 != a.end();
                      it2++ ) {
                    _code_to_error_page[HTTP::Response::code_to_string().at(
                        it2->unwrap< JSON::String >() )]
                        = it->first;
                }
            }
        }
    } catch ( const std::out_of_range & ) {
        throw ConfigError();
    } catch ( const std::bad_cast & ) { throw ConfigError(); }
}

const sockaddr_in &ServerConf::addr() const { return _addr; }

const std::set< std::string > &ServerConf::names() const { return _names; }

const ServerConf::RouteMapper &ServerConf::route_mapper() const {
    return _route_mapper;
}

const std::map< HTTP::Response::e_error_code, std::string > &
ServerConf::code_to_error_page() const {
    return _code_to_error_page;
}

size_t ServerConf::client_max_body_size() const {
    return _client_max_body_size;
}

/* -------------------------------------------------------------------------- */
