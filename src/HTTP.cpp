#include "HTTP.hpp"

/* --------------------------------- Request -------------------------------- */

const std::string &HTTP::Request::key_to_string( e_header_key k ) {
    typedef std::map< e_header_key, std::string > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[CONTENT_LENGTH]    = "Content-Length";
            m[TRANSFER_ENCODING] = "Transfer-Encoding";
            m[COOKIE]            = "Cookie";
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
            m[key_to_string( CONTENT_LENGTH )]    = CONTENT_LENGTH;
            m[key_to_string( TRANSFER_ENCODING )] = TRANSFER_ENCODING;
            m[key_to_string( COOKIE )]            = COOKIE;
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
      _request( new Request() ),
      _chunked( false ) {}

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
        if ( _request->_defined_header.count( TRANSFER_ENCODING )
             && _request->_defined_header.at( TRANSFER_ENCODING )
                    == "chunked" ) {
            _chunked = true;
            _step    = CONTENT;
        } else if ( _request->_defined_header.count( CONTENT_LENGTH ) ) {
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
        iss >> k;
        k = k.substr( 0, k.size() - 1 );
        v = iss.str().substr( k.size() + 2, iss.str().size() );
        if ( Request::string_to_key().count( k ) ) {
            _request->_defined_header[Request::string_to_key().at( k )] = v;
        } else {
            _request->_header[k] = v;
        }
    }
}

void HTTP::Request::DynamicParser::_append_to_content( const char *s,
                                                       size_t      n ) {
    if ( _chunked ) {
        _request->_content.append( s, n );
        if ( Str::ends_with( _request->_content, "\r\n\r\n" ) ) {
            _unchunk();
            _step = DONE;
        }
    } else {
        n = std::min( n, _content_length - _request->_content.size() );
        _request->_content.append( s, n );
        if ( _request->_content.size() >= _content_length ) { _step = DONE; }
    }
}

void HTTP::Request::DynamicParser::_unchunk() {
    std::string        new_content;
    std::istringstream chunked_stream( _request->_content );
    std::string        chunk_size_line;
    while ( std::getline( chunked_stream, chunk_size_line ) ) {
        chunk_size_line = Str::trim_right( chunk_size_line, "\r\n" );
        std::istringstream iss( chunk_size_line );
        size_t             chunk_size;
        iss >> chunk_size;
        std::string chunk_data( chunk_size, ' ' );
        chunked_stream.read( &chunk_data[0], chunk_size );
        new_content.append( chunk_data );
        std::string line_break;
        std::getline( chunked_stream, line_break );
    }
    _request->_content = new_content;
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

/* --------------------------- RequestHandler::CGI -------------------------- */

const std::string &HTTP::RequestHandler::CGI::env_key_to_string(
    HTTP::RequestHandler::CGI::e_env_key key ) {
    typedef std::map< e_env_key, std::string > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[PATH_INFO]       = "PATH_INFO";
            m[REQUEST_METHOD]  = "REQUEST_METHOD";
            m[CONTENT_TYPE]    = "CONTENT_TYPE";
            m[CONTENT_LENGTH]  = "CONTENT_LENGTH";
            m[QUERY_STRING]    = "QUERY_STRING";
            m[REDIRECT_STATUS] = "REDIRECT_STATUS";
            m[SCRIPT_NAME]     = "SCRIPT_NAME";
            m[SCRIPT_FILENAME] = "SCRIPT_FILENAME";
            return m;
        }
    };
    static const map_type m( f::init() );
    return m.at( key );
}

/* ------------------------ RequestHandler::CGI::Env ------------------------ */

char **HTTP::RequestHandler::CGI::Env::c_arr() const {
    char **env = new char *[size() + 1];
    size_t i( 0 );
    for ( const_iterator it( begin() ); it != end(); it++, i++ ) {
        env[i]
            = Str::dup( ( env_key_to_string( it->first ) ) + "=" + it->second );
    }
    env[size()] = 0;
    return env;
}

void HTTP::RequestHandler::CGI::Env::clear_c_env( char **envp ) {
    for ( char **p( envp ); *p; p++ ) { delete[] * p; }
    delete[] envp;
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

std::string HTTP::RequestHandler::make_raw_response() {
    if ( !_route ) { return make_error_response( Response::E404 ).stringify(); }
    if ( !_route->methods().count(
             Request::method_to_string( _request->method() ) ) ) {
        return make_error_response( Response::E405 ).stringify();
    }
    for ( std::map< std::string, std::string >::const_iterator it
          = _route->cgis().begin();
          it != _route->cgis().end();
          it++ ) {
        if ( Str::ends_with( _path, it->first ) ) { return _cgi( it->second ); }
    }
    if ( _route->redir().size() ) { return _redir().stringify(); }
    switch ( _request->method() ) {
    case Request::GET: return _get().stringify();
    case Request::POST: return _post().stringify();
    case Request::DELETE: return _delete().stringify();
    }
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

HTTP::Response HTTP::RequestHandler::_autoindex() {
    struct dirent *file;
    std::string    content    = "<!DOCTYPE html><html><body><h1>";
    std::string    contentEnd = "</h1></body></html>";
    DIR           *dir;
    dir = opendir( _path.c_str() );
    if ( !dir ) return make_error_response( Response::E500 );
    while ( ( file = readdir( dir ) ) != NULL ) {
        content += std::string( "<p><a href='" )
                   + _conf->route_mapper().route_name( _request->url() ) + '/'
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

std::string HTTP::RequestHandler::_cgi( const std::string &bin_path ) {
    std::string bp( bin_path );
    std::string p( _path );
    char       *args[] = { const_cast< char       *>( bp.c_str() ),
                           const_cast< char       *>( p.c_str() ),
                           0 };
    CGI::Env    env;
    env[CGI::PATH_INFO]      = _path;
    env[CGI::REQUEST_METHOD] = Request::method_to_string( _request->method() );
    if ( _request->header().count( "Content-Type" ) ) {
        env[CGI::CONTENT_TYPE] = _request->header().at( "Content-Type" );
    }
    env[CGI::CONTENT_LENGTH]  = Str::from( _request->content().size() );
    env[CGI::QUERY_STRING]    = "";
    env[CGI::REDIRECT_STATUS] = "200";
    env[CGI::SCRIPT_FILENAME] = _path;
    int i_pipe[2];
    int o_pipe[2];
    ::pipe( i_pipe );
    ::pipe( o_pipe );
    int pid = ::fork();
    if ( !pid ) {
        char **envp( env.c_arr() );
        ::dup2( i_pipe[0], STDIN_FILENO );
        ::close( i_pipe[1] );
        ::dup2( o_pipe[1], STDOUT_FILENO );
        ::close( o_pipe[0] );
        ::execve( *args, args, envp );
        ::exit( EXIT_FAILURE );
        CGI::Env::clear_c_env( envp );
    }
    close( i_pipe[0] );
    close( o_pipe[1] );
    write( i_pipe[1], _request->content().c_str(), _request->content().size() );
    close( i_pipe[1] );
    char        buff[1024];
    size_t      n;
    std::string s;
    while ( ( n = ::read( o_pipe[0], buff, 1024 ) ) == 1024 ) {
        s.append( buff, n );
    }
    close( o_pipe[0] );
    s.append( buff, n );
    wait( 0 );
    return Str::trim_right( Response( Response::E200 ).stringify(), "\r\n" )
           + "\r\n" + s;
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
