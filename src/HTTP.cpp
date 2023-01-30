#include "HTTP.hpp"

/* -------------------------------------------------------------------------- */

const std::map< int, std::string > &HTTP::Values::error_code_to_message() {
    struct f {
        static std::map< int, std::string > init() {
            std::map< int, std::string > m;
            m[200] = "OK";
            m[301] = "Move Permanently";
            m[400] = "Bad Request";
            m[403] = "Forbidden";
            m[404] = "Not Found";
            m[405] = "Method Not Allowed";
            m[408] = "Request Timeout";
            m[500] = "InternalServer Error";
            m[502] = "Bad Gateway";
            m[505] = "Version Not Supported";
            return m;
        }
    };
    static const std::map< int, std::string > m( f::init() );
    return m;
}

/* -------------------------------------------------------------------------- */

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
    switch ( _step ) {
    case REQUEST:
        iss >> _request->method;
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

/* -------------------------------------------------------------------------- */

std::string HTTP::Response::stringify() const {
    std::string s( version + ' ' + code + ' ' + outcome + "\r\n" );
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

/* -------------------------------------------------------------------------- */

HTTP::RequestHandler::RequestHandler( Ptr::shared< Request >    request,
                                      Ptr::shared< ServerConf > conf )
    : _request( request ),
      _conf( conf ),
      _route( 0 ) {
    if ( _conf->route_mapper().count( _request->url ) ) {
        _route = &_conf->route_mapper().at( _request->url );
        _path  = _route->root() + _conf->route_mapper().suffix( _request->url );
    }
    response();
}

void HTTP::RequestHandler::response() {
    if ( _request->version != "HTTP/1.1" ) {
        return setResponse( 505, errorMessage( 505 ) );
    }
    if ( !_route ) { return setResponse( 404, errorMessage( 404 ) ); }
    if ( _route->redir() != "" ) { return toRedir(); }
    std::map< std::string, void ( HTTP::RequestHandler::* )() > handler;
    handler["GET"]    = &RequestHandler::getMethod;
    handler["POST"]   = &RequestHandler::postMethod;
    handler["DELETE"] = &RequestHandler::deleteMethod;
    if ( _route->methods().count( _request->method ) ) {
        return ( this->*handler[_request->method] )();
    }
    setResponse( 405, errorMessage( 405 ) );
}

void HTTP::RequestHandler::getMethod() {
    struct stat s;
    if ( stat( _path.c_str(), &s ) ) {
        return errno == ENOENT ? setResponse( 404, errorMessage( 404 ) )
                               : setResponse( 500, errorMessage( 500 ) );
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
                return setResponse( 200, oss.str() );
            }
        }
        if ( _route->autoindex() ) { return toDirListing(); }
        return setResponse( 404, errorMessage( 404 ) );
    }
    std::ifstream f( _path.c_str() );
    if ( !f.is_open() ) { return setResponse( 404, errorMessage( 404 ) ); }
    std::ostringstream oss;
    oss << f.rdbuf();
    setResponse( 200, oss.str() );
}

void HTTP::RequestHandler::postMethod() {}

void HTTP::RequestHandler::deleteMethod() {
    struct stat s;
    if ( stat( _path.c_str(), &s ) == 0 ) {
        if ( s.st_mode & S_IFDIR ) {
            setResponse( 400, errorMessage( 400 ) );
        } else {
            std::remove( _path.c_str() );
            setResponse( 200, "" );
        }
    } else {
        if ( errno == ENOENT )
            setResponse( 404, errorMessage( 404 ) );
        else {
            setResponse( 500, errorMessage( 500 ) );
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
    setResponse( 200, content );
}

void HTTP::RequestHandler::toRedir() {
    std::string content = "<meta http-equiv=\"refresh\" content=\"0;URL="
                          + _route->redir() + "\"/>";
    _response.header[Response::CONTENT_TYPE] = _conf->mime().at( "html" );
    setResponse( 301, content );
}

void HTTP::RequestHandler::setResponse( int nb, std::string content ) {
    _response.code    = Str::from( nb );
    _response.outcome = HTTP::Values::error_code_to_message().at( nb );
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

std::string HTTP::RequestHandler::errorMessage( int nb ) {
    std::string content;

    content = "<!DOCTYPE html><html>";
    content += "<h1>" + Str::from( nb ) + "</h>";
    content += "<p>" + HTTP::Values::error_code_to_message().at( nb )
               + "</p></html>";
    return ( content );
}

HTTP::Response HTTP::RequestHandler::getResponse() { return ( _response ); }

std::string HTTP::RequestHandler::getContentType( const std::string &path ) {
    size_t found ( path.find_last_of( '.' ));
    if ( found == std::string::npos ) return ( "text/plain" );
    std::string last( path, found + 1 );
    if ( !_conf->mime().count( last ) ) { return "text/plain"; }
    return _conf->mime().at( last );
}

/* -------------------------------------------------------------------------- */
