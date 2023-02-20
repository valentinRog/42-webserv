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
            m.insert( std::make_pair( HANDLER, "handler" ) );
            return m;
        }
    };
    static const BiMap< e_config_key, std::string > m( f::init() );
    return m;
}

ServerConf::Route::Route() : _autoindex( false ) {}

Option< ServerConf::Route >
ServerConf::Route::from_json( const JSON::Object &o ) {
    Route           r;
    Option< Route > fail;
    for ( JSON::Object::const_iterator it( o.begin() ); it != o.end(); it++ ) {
        if ( !key_to_string().count( it->first ) ) { return fail; }
    }
    {
        if ( !o.count( key_to_string().at( ROOT ) ) ) { return fail; }
        const JSON::String *p
            = o.at( key_to_string().at( ROOT ) ).dycast< JSON::String >();
        if ( !p ) { return fail; }
        r._root = *p;
    }
    if ( o.count( key_to_string().at( INDEX ) ) ) {
        const JSON::String *p
            = o.at( key_to_string().at( INDEX ) ).dycast< JSON::String >();
        if ( !p ) { return fail; }
        r._index = *p;
    }
    if ( o.count( key_to_string().at( AUTOINDEX ) ) ) {
        const JSON::Boolean *p
            = o.at( key_to_string().at( AUTOINDEX ) ).dycast< JSON::Boolean >();
        if ( !p ) { return fail; }
        r._autoindex = *p;
    }
    if ( o.count( key_to_string().at( METHODS ) ) ) {
        const JSON::Array *p
            = o.at( key_to_string().at( METHODS ) ).dycast< JSON::Array >();
        if ( !p ) { return fail; }
        for ( JSON::Array::const_iterator it( p->begin() ); it != p->end();
              it++ ) {
            const JSON::String *p = it->dycast< JSON::String >();
            if ( !p ) { return fail; }
            r._methods.insert( *p );
        }
    }
    if ( o.count( key_to_string().at( REDIR ) ) ) {
        const JSON::String *p
            = o.at( key_to_string().at( REDIR ) ).dycast< JSON::String >();
        if ( !p ) { return fail; }
        r._redir = *p;
    }
    if ( o.count( key_to_string().at( CGI ) ) ) {
        const JSON::Object *p
            = o.at( key_to_string().at( CGI ) ).dycast< JSON::Object >();
        if ( !p ) { return fail; }
        for ( JSON::Object::const_iterator it = p->begin(); it != p->end();
              it++ ) {
            const JSON::String *p = it->second.dycast< JSON::String >();
            if ( !p ) { return fail; }
            r._cgis[it->first] = *p;
        }
    }
    if ( o.count( key_to_string().at( HANDLER ) ) ) {
        const JSON::String *p
            = o.at( key_to_string().at( HANDLER ) ).dycast< JSON::String >();
        if ( !p ) { return fail; }
        r._handler = *p;
    }
    return r;
}

const std::string &ServerConf::Route::root() const { return _root; }

const std::string &ServerConf::Route::index() const { return _index; }

const std::set< std::string > &ServerConf::Route::methods() const {
    return _methods;
}

bool ServerConf::Route::autoindex() const { return _autoindex; }

const std::string &ServerConf::Route::redir() const { return _redir; }

const std::map< std::string, std::string > &ServerConf::Route::cgis() const {
    return _cgis;
}

const std::string &ServerConf::Route::handler() const { return _handler; }

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

/* ------------------------------- ServerConf ------------------------------- */

ServerConf::ServerConf() : _client_max_body_size( SIZE_MAX ) {}

const BiMap< ServerConf::e_config_key, std::string > &
ServerConf::key_to_string() {
    struct f {
        static BiMap< e_config_key, std::string > init() {
            typedef std::pair< e_config_key, std::string > value_type;
            BiMap< e_config_key, std::string >             m;
            m.insert( value_type( LISTEN, "listen" ) );
            m.insert( value_type( SERVER_NAMES, "names" ) );
            m.insert(
                value_type( CLIENT_MAX_BODY_SIZE, "client_max_body_size" ) );
            m.insert( value_type( ROUTES, "routes" ) );
            m.insert( value_type( ERROR_PAGES, "error_pages" ) );
            return m;
        }
    };
    static const BiMap< e_config_key, std::string > m( f::init() );
    return m;
}

Option< ServerConf > ServerConf::from_json( const JSON::Object &o ) {
    ServerConf           r;
    Option< ServerConf > fail;
    for ( JSON::Object::const_iterator it( o.begin() ); it != o.end(); it++ ) {
        if ( !key_to_string().count( it->first ) ) { return fail; }
    }
    ::bzero( &r._addr, sizeof r._addr );
    r._addr.sin_family = AF_INET;
    {
        const JSON::Array *p
            = o.at( key_to_string().at( LISTEN ) ).dycast< JSON::Array >();
        if ( !p ) { return fail; }
        {
            const JSON::Number *p2 = ( *p )[1].dycast< JSON::Number >();
            if ( !p2 ) { return fail; }
            if ( *p2 > 65535 || *p2 <= 0 ) { return fail; }
            r._addr.sin_port = htons( *p2 );
        }
        {
            const JSON::String *p2 = ( *p )[0].dycast< JSON::String >();
            if ( !p2 ) { return fail; }
            if ( ( r._addr.sin_addr.s_addr
                   = ::inet_addr( std::string( *p2 ).c_str() ) )
                 == INADDR_NONE ) {
                return fail;
            }
        }
    }
    if ( o.count( key_to_string().at( SERVER_NAMES ) ) ) {
        const JSON::Array *p = o.at( key_to_string().at( SERVER_NAMES ) )
                                   .dycast< JSON::Array >();
        if ( !p ) { return fail; }
        for ( JSON::Array::const_iterator it( p->begin() ); it != p->end();
              it++ ) {
            const JSON::String *p = it->dycast< JSON::String >();
            if ( !p ) { return fail; }
            r._names.insert( *p );
        }
    }
    {
        const JSON::Object *p
            = o.at( key_to_string().at( ROUTES ) ).dycast< JSON::Object >();
        if ( !p ) { return fail; }
        for ( JSON::Object::const_iterator it( p->begin() ); it != p->end();
              it++ ) {
            const JSON::Object *p = it->second.dycast< JSON::Object >();
            if ( !p ) { return fail; }
            Option< Route > route = Route::from_json( *p );
            if ( route.is_none() ) { return fail; }
            r._route_mapper.insert(
                std::make_pair( it->first, route.unwrap() ) );
        }
    }
    if ( o.count( key_to_string().at( CLIENT_MAX_BODY_SIZE ) ) ) {
        const JSON::Number *p
            = o.at( key_to_string().at( CLIENT_MAX_BODY_SIZE ) )
                  .dycast< JSON::Number >();
        if ( !p ) { return fail; }
        r._client_max_body_size = *p;
    }
    if ( o.count( key_to_string().at( ERROR_PAGES ) ) ) {
        const JSON::Object *p = o.at( key_to_string().at( ERROR_PAGES ) )
                                    .dycast< JSON::Object >();
        if ( !p ) { return fail; }
        for ( JSON::Object::const_iterator it( p->begin() ); it != p->end();
              it++ ) {
            const JSON::Array *p = it->second.dycast< JSON::Array >();
            if ( !p ) { return fail; }
            for ( JSON::Array::const_iterator it2( p->begin() );
                  it2 != p->end();
                  it2++ ) {
                const JSON::String *p = it2->dycast< JSON::String >();
                if ( !p ) { return fail; }
                r._code_to_error_page[HTTP::Response::code_to_string().at( *p )]
                    = it->first;
            }
        }
    }
    return r;
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
