#include "HTTP.hpp"

/* --------------------------------- Request -------------------------------- */

const std::string &HTTP::Request::key_to_string( e_header_key k ) {
    typedef std::map< e_header_key, std::string > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[CONTENT_LENGTH] = "Content-Length";
            return m;
        }
    };
    static const map_type m( f::init() );
    return m.at( k );
}

const std::
    map< std::string, HTTP::Request::e_header_key, Str::CaseInsensitiveCmp > &
    HTTP::Request::string_to_key() {
    typedef std::map< std::string, e_header_key, Str::CaseInsensitiveCmp >
        map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[key_to_string( CONTENT_LENGTH )] = CONTENT_LENGTH;
            return m;
        }
    };
    static const map_type m( f::init() );
    return m;
}

const std::string &
HTTP::Request::method_to_string( HTTP::Request::e_method method ) {
    typedef std::map< e_method, std::string > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[GET]    = "GET";
            m[POST]   = "POST";
            m[DELETE] = "DELETE";
            m[PUT]    = "PUT";
            return m;
        }
    };
    static const map_type m( f::init() );
    return m.at( method );
}

const std::map< std::string, HTTP::Request::e_method > &
HTTP::Request::string_to_method() {
    typedef std::map< std::string, e_method > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[method_to_string( GET )]    = GET;
            m[method_to_string( POST )]   = POST;
            m[method_to_string( DELETE )] = DELETE;
            m[method_to_string( PUT )]    = PUT;
            return m;
        }
    };
    static const map_type m( f::init() );
    return m;
}

HTTP::Request::e_method HTTP::Request::method() const { return _method; }

const std::string &HTTP::Request::url() const { return _url; }

const std::string &HTTP::Request::version() const { return _version; }

const std::string &HTTP::Request::host() const { return _host; }

const std::map< HTTP::Request::e_header_key, std::string > &
HTTP::Request::defined_header() const {
    return _defined_header;
}

const std::map< std::string, std::string, Str::CaseInsensitiveCmp > &
HTTP::Request::header() const {
    return _header;
}

const std::string &HTTP::Request::content() const { return _content; }

/* ------------------------- Request::DynamicParser ------------------------- */

HTTP::Request::DynamicParser::DynamicParser()
    : _step( REQUEST ),
      _request( new Request() ) {}

void HTTP::Request::DynamicParser::add( const char *s, size_t n ) {
    if ( _step & ( DONE | FAILED ) ) { return; }
    const char *p( s );
    if ( _step != CONTENT ) {
        for ( ; p < s + n && !( _step & ( CONTENT | FAILED ) ); p++ ) {
            if ( *p == '\r' || *p == '\n' ) {
                _sep += *p;
                if ( _sep == "\r\n" ) {
                    _parse_line();
                    _line.clear();
                    _sep.clear();
                }
            } else {
                _line += *p;
            }
        }
    }
    if ( _step == CONTENT ) { _append_to_content( p, n - ( p - s ) ); }
}

HTTP::Request::DynamicParser::e_step
HTTP::Request::DynamicParser::step() const {
    return _step;
}

Ptr::Shared< HTTP::Request > HTTP::Request::DynamicParser::request() {
    return _request;
}

void HTTP::Request::DynamicParser::_parse_line() {
    switch ( _step ) {
    case REQUEST: _parse_request_line(); break;
    case HOST: _parse_host_line(); break;
    case HEADER: _parse_header_line(); break;
    default: break;
    }
}

void HTTP::Request::DynamicParser::_parse_request_line() {
    std::istringstream iss( _line );
    std::string        s;
    iss >> s;
    _request->_method = Request::string_to_method().at( s );
    iss >> _request->_url;
    iss >> _request->_version;
    _step = HOST;
}

void HTTP::Request::DynamicParser::_parse_host_line() {
    std::istringstream iss( _line );
    iss.ignore( std::numeric_limits< std::streamsize >::max(), ' ' );
    iss >> _request->_host;
    _step = HEADER;
}

void HTTP::Request::DynamicParser::_parse_header_line() {
    std::istringstream iss( _line );
    if ( !iss.str().size() ) {
        if ( _request->_defined_header.count( CONTENT_LENGTH ) ) {
            std::istringstream iss(
                _request->_defined_header.at( CONTENT_LENGTH ) );
            iss >> _content_length;
            _step = CONTENT;
        } else {
            _step = DONE;
        }
    } else {
        std::string k;
        std::string v;
        iss >> k >> v;
        k = k.substr( 0, k.size() - 1 );
        if ( Request::string_to_key().count( k ) ) {
            _request->_defined_header[Request::string_to_key().at( k )] = v;
        } else {
            _request->_header[k] = v;
        }
    }
}

void HTTP::Request::DynamicParser::_append_to_content( const char *s,
                                                       size_t      n ) {
    _request->_content.append(
        s,
        std::min( n, _content_length - _request->_content.size() ) );
    if ( _request->_content.size() >= _content_length ) { _step = DONE; }
}

/* -------------------------------- Response -------------------------------- */

const std::pair< std::string, std::string > &
HTTP::Response::error_code_to_string( HTTP::Response::e_error_code code ) {
    struct f {
        static std::map< e_error_code, std::pair< std::string, std::string > >
        init() {
            std::map< e_error_code, std::pair< std::string, std::string > > m;
            m[E200] = std::make_pair( "200", "OK" );
            m[E301] = std::make_pair( "301", "Move Permanently" );
            m[E400] = std::make_pair( "400", "Bad Request" );
            m[E403] = std::make_pair( "403", "Forbidden" );
            m[E404] = std::make_pair( "404", "Not Found" );
            m[E405] = std::make_pair( "405", "Method Not Allowed" );
            m[E408] = std::make_pair( "408", "Request Timeout" );
            m[E500] = std::make_pair( "500", "InternalServer Error" );
            m[E502] = std::make_pair( "502", "Bad Gateway" );
            m[E505] = std::make_pair( "505", "Version Not Supported" );
            return m;
        }
    };
    static const std::map< e_error_code, std::pair< std::string, std::string > >
        m( f::init() );
    return m.at( code );
}

const std::string &HTTP::Response::version() {
    static const std::string s( "HTTP/1.1" );
    return s;
}

std::string HTTP::Response::stringify() const {
    std::string s( version() + ' ' + error_code_to_string( code ).first + ' '
                   + error_code_to_string( code ).second + "\r\n" );
    for ( std::map< e_header_key, std::string >::const_iterator it
          = header.begin();
          it != header.end();
          it++ ) {
        s += _header_key_name().at( it->first ) + ": " + it->second + "\r\n";
    }
    return s + "\r\n" + _content;
}

const std::map< HTTP::Response::e_header_key, std::string > &
HTTP::Response::_header_key_name() {
    struct f {
        static std::map< e_header_key, std::string > init() {
            std::map< e_header_key, std::string > m;
            m[HOST]           = "Host";
            m[CONTENT_TYPE]   = "Content-Type";
            m[CONTENT_LENGTH] = "Content-Length";
            m[LOCATION]       = "Location";
            return m;
        }
    };
    static const std::map< e_header_key, std::string > m( f::init() );
    return m;
}

/* ----------------------------- RequestHandler ----------------------------- */

HTTP::RequestHandler::RequestHandler( Ptr::Shared< Request >    request,
                                      Ptr::Shared< ServerConf > conf )
    : _request( request ),
      _conf( conf ),
      _route( 0 ) {
    if ( _conf->route_mapper().count( _request->url() + '/' ) ) {
        _route = &_conf->route_mapper().at( _request->url() + '/' );
        _path  = _route->root() + '/'
                + _conf->route_mapper().suffix( _request->url() );
    }
}

HTTP::Response
HTTP::RequestHandler::make_error_response( HTTP::Response::e_error_code code ) {
    Response r( code );
    r.set_content( "<h1>" + HTTP::Response::error_code_to_string( code ).first
                   + "</h1>" );
    return r;
}

HTTP::Response HTTP::RequestHandler::make_response() {
    if ( !_route ) { return make_error_response( Response::E404 ); }
    if ( _route->redir().size() ) { return _redir(); }
    if ( _route->methods().count(
             Request::method_to_string( _request->method() ) ) ) {
        switch ( _request->method() ) {
        case Request::GET: return _get();
        case Request::POST: return _post();
        case Request::DELETE: return _delete();
        case Request::PUT: return _put();
        }
    }
    return make_error_response( Response::E405 );
}

HTTP::Response HTTP::RequestHandler::_get() {
    struct stat s;
    if ( stat( _path.c_str(), &s ) ) {
        return errno == ENOENT ? make_error_response( Response::E404 )
                               : make_error_response( Response::E500 );
    }
    if ( s.st_mode & S_IFDIR ) {
        for ( std::list< std::string >::const_iterator it(
                  _route->index().begin() );
              it != _route->index().end();
              it++ ) {
            std::ifstream f( ( _path + '/' + *it ).c_str() );
            if ( f.is_open() ) {
                _path += '/' + *it;
                Response r( Response::E200 );
                r.set_content(
                    std::string( ( std::istreambuf_iterator< char >( f ) ),
                                 std::istreambuf_iterator< char >() ) );
                r.header[Response::CONTENT_TYPE] = _content_type( *it );
                return r;
            }
        }
        if ( _route->autoindex() ) { return _autoindex(); }
        return make_error_response( Response::E404 );
    }
    std::ifstream f( _path.c_str() );
    if ( !f.is_open() ) { return make_error_response( Response::E404 ); }
    Response r( Response::E200 );
    r.set_content( std::string( ( std::istreambuf_iterator< char >( f ) ),
                                std::istreambuf_iterator< char >() ) );
    r.header[Response::CONTENT_TYPE] = _content_type( _path );
    return r;
}

HTTP::Response HTTP::RequestHandler::_post() {
    return Response( Response::E500 );
}

HTTP::Response HTTP::RequestHandler::_delete() {
    struct stat s;
    if ( stat( _path.c_str(), &s ) == 0 ) {
        if ( s.st_mode & S_IFDIR ) {
            return Response( Response::E400 );
        } else {
            std::remove( _path.c_str() );
            return Response( Response::E200 );
        }
    }
    return errno == ENOENT ? Response( Response::E404 )
                           : Response( Response::E500 );
}

HTTP::Response HTTP::RequestHandler::_put() {
    std::ofstream f( _path.c_str() );
    if ( f.is_open() ) {
        f << _request->content();
        return Response( Response::E200 );
    }
    return Response( Response::E404 );
}

HTTP::Response HTTP::RequestHandler::_autoindex() {
    struct dirent *file;
    std::string    content    = "<!DOCTYPE html><html><body><h1>";
    std::string    contentEnd = "</h1></body></html>";
    DIR *          dir;
    dir = opendir( _path.c_str() );
    if ( !dir ) return make_error_response( Response::E500 );
    while ( ( file = readdir( dir ) ) != NULL ) {
        content += std::string( "<p><a href='" ) 
                   + _conf->route_mapper().suffix( _request->url() ) + '/'
                   + file->d_name + "'>" + file->d_name + "</a></p>";
    }
    content += contentEnd;
    closedir( dir );
    Response r( Response::E200 );
    r.set_content( content );
    r.header[Response::CONTENT_TYPE] = _content_type( ".html" );
    return r;
}

HTTP::Response HTTP::RequestHandler::_redir() {
    Response r( Response::E301 );
    r.header[Response::LOCATION] = _route->redir();
    return r;
}

const std::string &
HTTP::RequestHandler::_content_type( const std::string &path ) const {
    static const std::string default_type( "text/plain" );
    size_t                   found( path.find_last_of( '.' ) );
    if ( found == std::string::npos ) return ( default_type );
    std::string last( path, found + 1 );
    if ( !_conf->mime().count( last ) ) { return default_type; }
    return _conf->mime().at( last );
}

/* -------------------------------------------------------------------------- */
