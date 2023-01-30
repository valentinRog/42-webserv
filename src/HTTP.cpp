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

const std::string &
HTTP::Request::method_to_string( HTTP::Request::e_method method ) {
    typedef std::map< e_method, std::string > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[GET]    = "GET";
            m[POST]   = "POST";
            m[DELETE] = "DELETE";
            return m;
        }
    };
    static const map_type m( f::init() );
    return m.at( method );
}

HTTP::Request::e_method
HTTP::Request::string_to_method( const std::string &s ) {
    typedef std::map< std::string, e_method > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m["GET"]    = GET;
            m["POST"]   = POST;
            m["DELETE"] = DELETE;
            return m;
        }
    };
    static const map_type m( f::init() );
    return m.at( s );
}

/* ------------------------- Request::DynamicParser ------------------------- */

HTTP::Request::DynamicParser::DynamicParser()
    : _step( REQUEST ),
      _request( new Request() ) {}

void HTTP::Request::DynamicParser::operator<<( const std::string &s ) {
    for ( std::string::const_iterator it( s.begin() ); it != s.end(); ++it ) {
        if ( *it == '\r' || *it == '\n' ) {
            _sep += *it;
            if ( _sep == "\r\n" ) {
                _parse_line();
                _line.clear();
                _sep.clear();
            }
        } else {
            _line += *it;
        }
    }
}

HTTP::Request::DynamicParser::e_step
HTTP::Request::DynamicParser::step() const {
    return _step;
}

Ptr::shared< HTTP::Request > HTTP::Request::DynamicParser::request() {
    return _request;
}

void HTTP::Request::DynamicParser::_parse_line() {
    std::istringstream iss( _line );
    std::string        s;
    switch ( _step ) {
    case REQUEST:
        iss >> s;
        _request->method = HTTP::Request::string_to_method( s );
        iss >> _request->url;
        iss >> _request->version;
        _step = HOST;
        break;
    case HOST:
        iss.ignore( std::numeric_limits< std::streamsize >::max(), ' ' );
        iss >> _request->host;
        _step = HEADER;
        break;
    case HEADER: {
        if ( !iss.str().size() ) {
            _step = CONTENT;
        } else {
            std::string k;
            std::string v;
            iss >> k >> v;
            k                   = k.substr( 0, k.size() - 1 );
            _request->header[k] = v;
            break;
        }
    }
    case CONTENT: _step = DONE;
    case DONE: break;
    }
}

/* -------------------------------- Response -------------------------------- */

const std::pair< int, std::string > &
HTTP::Response::error_code_to_string( HTTP::Response::e_error_code code ) {
    struct f {
        static std::map< e_error_code, std::pair< int, std::string > > init() {
            std::map< e_error_code, std::pair< int, std::string > > m;
            m[E200] = std::make_pair( 200, "OK" );
            m[E301] = std::make_pair( 301, "Move Permanently" );
            m[E400] = std::make_pair( 400, "Bad Request" );
            m[E403] = std::make_pair( 403, "Forbidden" );
            m[E404] = std::make_pair( 404, "Not Found" );
            m[E405] = std::make_pair( 405, "Method Not Allowed" );
            m[E408] = std::make_pair( 408, "Request Timeout" );
            m[E500] = std::make_pair( 500, "InternalServer Error" );
            m[E502] = std::make_pair( 502, "Bad Gateway" );
            m[E505] = std::make_pair( 505, "Version Not Supported" );
            return m;
        }
    };
    static const std::map< e_error_code, std::pair< int, std::string > > m(
        f::init() );
    return m.at( code );
}

std::string HTTP::Response::stringify() const {
    std::string s( version + ' '
                   + Str::from( error_code_to_string( code ).first ) + ' '
                   + error_code_to_string( code ).second + "\r\n" );
    for ( std::map< e_header_key, std::string >::const_iterator it
          = header.begin();
          it != header.end();
          it++ ) {
        s += _header_key_name().at( it->first ) + ": " + it->second + "\r\n";
    }
    return s + "\r\n" + content;
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

HTTP::RequestHandler::RequestHandler( Ptr::shared< Request >    request,
                                      Ptr::shared< ServerConf > conf )
    : _request( request ),
      _conf( conf ),
      _route( 0 ) {
    if ( _conf->route_mapper().count( _request->url ) ) {
        _route = &_conf->route_mapper().at( _request->url );
        _path  = _route->root() + '/'
                + _conf->route_mapper().suffix( _request->url );
    }
    response();
}

void HTTP::RequestHandler::response() {
    if ( _request->version != "HTTP/1.1" ) {
        return setResponse( Response::E505, errorMessage( Response::E505 ) );
    }
    if ( !_route ) {
        return setResponse( Response::E404, errorMessage( Response::E404 ) );
    }
    if ( _route->redir() != "" ) { return toRedir(); }
    std::map< Request::e_method, void ( HTTP::RequestHandler::* )() > handler;
    handler[Request::GET]    = &RequestHandler::getMethod;
    handler[Request::POST]   = &RequestHandler::postMethod;
    handler[Request::DELETE] = &RequestHandler::deleteMethod;
    if ( _route->methods().count(
             Request::method_to_string( _request->method ) ) ) {
        return ( this->*handler[_request->method] )();
    }
    setResponse( Response::E405, errorMessage( Response::E405 ) );
}

void HTTP::RequestHandler::getMethod() {
    struct stat s;
    if ( stat( _path.c_str(), &s ) ) {
        return errno == ENOENT ? setResponse( Response::E404,
                                              errorMessage( Response::E404 ) )
                               : setResponse( Response::E500,
                                              errorMessage( Response::E500 ) );
    }
    if ( s.st_mode & S_IFDIR ) {
        for ( std::list< std::string >::const_iterator it(
                  _route->index().begin() );
              it != _route->index().end();
              it++ ) {
            std::cout << _path + '/' + *it << std::endl;
            std::ifstream f( ( _path + '/' + *it ).c_str() );
            if ( f.is_open() ) {
                _path += '/' + *it;
                std::ostringstream oss;
                oss << f.rdbuf();
                return setResponse( Response::E200, oss.str() );
            }
        }
        if ( _route->autoindex() ) { return toDirListing(); }
        return setResponse( Response::E404, errorMessage( Response::E404 ) );
    }
    std::ifstream f( _path.c_str() );
    if ( !f.is_open() ) {
        return setResponse( Response::E404, errorMessage( Response::E404 ) );
    }
    std::ostringstream oss;
    oss << f.rdbuf();
    setResponse( Response::E200, oss.str() );
}

void HTTP::RequestHandler::postMethod() {}

void HTTP::RequestHandler::deleteMethod() {
    struct stat s;
    if ( stat( _path.c_str(), &s ) == 0 ) {
        if ( s.st_mode & S_IFDIR ) {
            setResponse( Response::E400, errorMessage( Response::E400 ) );
        } else {
            std::remove( _path.c_str() );
            setResponse( Response::E200, "" );
        }
    } else {
        if ( errno == ENOENT )
            setResponse( Response::E404, errorMessage( Response::E404 ) );
        else {
            setResponse( Response::E500, errorMessage( Response::E500 ) );
        }
    }
}

void HTTP::RequestHandler::toDirListing() {
    struct dirent *file;
    std::string    content    = "<!DOCTYPE html><html><body><h1>";
    std::string    contentEnd = "</h1></body></html>";

    DIR *dir;
    dir = opendir( _path.c_str() );
    if ( !dir ) return;
    while ( ( file = readdir( dir ) ) != NULL )
        content += "<p>" + std::string( file->d_name ) + "</p>";
    content += contentEnd;
    closedir( dir );
    setResponse( Response::E200, content );
}

void HTTP::RequestHandler::toRedir() {
    std::string content = "<meta http-equiv=\"refresh\" content=\"0;URL="
                          + _route->redir() + "\"/>";
    _response.header[Response::CONTENT_TYPE] = _conf->mime().at( "html" );
    setResponse( Response::E301, content );
}

void HTTP::RequestHandler::setResponse( HTTP::Response::e_error_code code,
                                        std::string                  content ) {
    _response.code    = code;
    _response.version = _request->version;
    _response.content = content;
    if ( _route->redir().empty() )
        _response.header[Response::CONTENT_TYPE] = getContentType( _path );
    else {
        _response.header[Response::LOCATION] = _route->redir();
    }
    _response.header[Response::HOST] = "ddfdfdfd";
    if ( !content.empty() ) {
        _response.header[Response::CONTENT_LENGTH]
            = Str::from( content.size() );
    }
}

std::string
HTTP::RequestHandler::errorMessage( HTTP::Response::e_error_code code ) {
    std::string content;

    content = "<!DOCTYPE html><html>";
    content += "<h1>"
               + Str::from( Response::error_code_to_string( code ).first )
               + "</h>";
    content += "<p>" + Response::error_code_to_string( code ).second
               + "</p></html>";
    return ( content );
}

HTTP::Response HTTP::RequestHandler::getResponse() { return ( _response ); }

std::string HTTP::RequestHandler::getContentType( const std::string &path ) {
    size_t found( path.find_last_of( '.' ) );
    if ( found == std::string::npos ) return ( "text/plain" );
    std::string last( path, found + 1 );
    if ( !_conf->mime().count( last ) ) { return "text/plain"; }
    return _conf->mime().at( last );
}

/* -------------------------------------------------------------------------- */
