#include "ServerConf.hpp"

/* ---------------------------- ServerConf::Route --------------------------- */

const std::string &
ServerConf::Route::key_to_string( ServerConf::Route::e_config_key key ) {
    typedef std::map< e_config_key, std::string > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[ROOT]      = "root";
            m[INDEX]     = "index";
            m[METHODS]   = "methods";
            m[CGI]       = "cgi";
            m[AUTOINDEX] = "autoindex";
            m[REDIR]     = "redir";
            return m;
        }
    };
    static const map_type m( f::init() );
    return m.at( key );
}

const std::map< std::string, ServerConf::Route::e_config_key > &
ServerConf::Route::string_to_key() {
    typedef std::map< std::string, e_config_key > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[key_to_string( ROOT )]      = ROOT;
            m[key_to_string( INDEX )]     = INDEX;
            m[key_to_string( METHODS )]   = METHODS;
            m[key_to_string( CGI )]       = CGI;
            m[key_to_string( AUTOINDEX )] = AUTOINDEX;
            m[key_to_string( REDIR )]     = REDIR;
            return m;
        }
    };
    static const map_type m( f::init() );
    return m;
}

ServerConf::Route::Route( const JSON::Object &o ) : _autoindex( false ) {
    for ( JSON::Object::const_iterator it( o.begin() ); it != o.end(); it++ ) {
        string_to_key().at( it->first );
    }
    _root = o.at( key_to_string( ROOT ) ).unwrap< JSON::String >();
    if ( o.count( key_to_string( INDEX ) ) ) {
        JSON::Array a( o.at( key_to_string( INDEX ) ).unwrap< JSON::Array >() );
        for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
              it++ ) {
            _index.push_back( it->unwrap< JSON::String >() );
        }
    }
    if ( o.count( key_to_string( AUTOINDEX ) ) ) {
        _autoindex
            = o.at( key_to_string( AUTOINDEX ) ).unwrap< JSON::Boolean >();
    }
    if ( o.count( key_to_string( METHODS ) ) ) {
        JSON::Array a(
            o.at( key_to_string( METHODS ) ).unwrap< JSON::Array >() );
        for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
              it++ ) {
            _methods.insert( it->unwrap< JSON::String >() );
        }
    }
    if ( o.count( key_to_string( REDIR ) ) ) {
        _redir = o.at( key_to_string( REDIR ) ).unwrap< JSON::String >();
    }
    if ( o.count( key_to_string( CGI ) ) ) {
        JSON::Object cgi_o(
            o.at( key_to_string( CGI ) ).unwrap< JSON::Object >() );
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

const std::string &ServerConf::key_to_string( ServerConf::e_config_key key ) {
    typedef std::map< e_config_key, std::string > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[LISTEN]               = "listen";
            m[SERVER_NAMES]         = "names";
            m[CLIENT_MAX_BODY_SIZE] = "client_max_body_size";
            m[ROUTES]               = "routes";
            m[ERROR_PAGES]          = "error_pages";
            return m;
        }
    };
    static const map_type m( f::init() );
    return m.at( key );
}

const std::map< std::string, ServerConf::e_config_key > &
ServerConf::string_to_key() {
    typedef std::map< std::string, e_config_key > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[key_to_string( LISTEN )]               = LISTEN;
            m[key_to_string( SERVER_NAMES )]         = SERVER_NAMES;
            m[key_to_string( CLIENT_MAX_BODY_SIZE )] = CLIENT_MAX_BODY_SIZE;
            m[key_to_string( ROUTES )]               = ROUTES;
            m[key_to_string( ERROR_PAGES )]          = ERROR_PAGES;
            return m;
        }
    };
    static const map_type m( f::init() );
    return m;
}

ServerConf::ServerConf(
    const JSON::Object                                 &o,
    Ptr::Shared< std::map< std::string, std::string > > mime )
    : _mime( mime ),
      _client_max_body_size( std::numeric_limits< std::size_t >::max() ) {
    try {
        for ( JSON::Object::const_iterator it( o.begin() ); it != o.end();
              it++ ) {
            string_to_key().at( it->first );
        }
        ::bzero( &_addr, sizeof _addr );
        _addr.sin_family = AF_INET;
        JSON::Array a(
            o.at( key_to_string( LISTEN ) ).unwrap< JSON::Array >() );
        _addr.sin_port = htons( a[1].unwrap< JSON::Number >() );
        if ( ( _addr.sin_addr.s_addr = ::inet_addr(
                   std::string( a[0].unwrap< JSON::String >() ).c_str() ) )
             == INADDR_NONE ) {
            throw std::runtime_error( "inet_addr" );
        }
        if ( o.count( key_to_string( SERVER_NAMES ) ) ) {
            JSON::Array a(
                o.at( key_to_string( SERVER_NAMES ) ).unwrap< JSON::Array >() );
            for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
                  it++ ) {
                _names.insert( it->unwrap< JSON::String >() );
            }
        }
        JSON::Object routes_o(
            o.at( key_to_string( ROUTES ) ).unwrap< JSON::Object >() );
        for ( JSON::Object::const_iterator it( routes_o.begin() );
              it != routes_o.end();
              it++ ) {
            _route_mapper.insert(
                std::make_pair( it->first,
                                it->second.unwrap< JSON::Object >() ) );
        }
        if ( o.count( key_to_string( CLIENT_MAX_BODY_SIZE ) ) ) {
            _client_max_body_size
                = o.at( key_to_string( CLIENT_MAX_BODY_SIZE ) )
                      .unwrap< JSON::Number >();
        }
        if ( o.count( key_to_string( ERROR_PAGES ) ) ) {
            JSON::Object error_pages_o(
                o.at( key_to_string( ERROR_PAGES ) ).unwrap< JSON::Object >() );
            for ( JSON::Object::const_iterator it( error_pages_o.begin() );
                  it != error_pages_o.end();
                  it++ ) {
                JSON::Array a( it->second.unwrap< JSON::Array >() );
                for ( JSON::Array::const_iterator it2( a.begin() );
                      it2 != a.end();
                      it2++ ) {
                    _code_to_error_page[HTTP::Response::string_to_error_code()
                                            .at(
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

const std::map< std::string, std::string > &ServerConf::mime() const {
    return *_mime;
}

const std::map< HTTP::Response::e_error_code, std::string > &
ServerConf::code_to_error_page() const {
    return _code_to_error_page;
}

size_t ServerConf::client_max_body_size() const {
    return _client_max_body_size;
}

/* -------------------------------------------------------------------------- */
