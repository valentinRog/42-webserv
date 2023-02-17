#include "RequestHandler.hpp"

/* --------------------------- RequestHandler::CGI -------------------------- */

const std::string &
RequestHandler::CGI::key_to_string( RequestHandler::CGI::e_env_key k ) {
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
            m[HTTP_COOKIE]     = "HTTP_COOKIE";
            return m;
        }
    };
    static const map_type m( f::init() );
    return m.at( k );
}

/* ------------------------ RequestHandler::CGI::Env ------------------------ */

char **RequestHandler::CGI::Env::c_arr() const {
    char **env = new char *[size() + 1];
    size_t i( 0 );
    for ( const_iterator it( begin() ); it != end(); it++, i++ ) {
        env[i] = Str::dup( ( key_to_string( it->first ) ) + "=" + it->second );
    }
    env[size()] = 0;
    return env;
}

void RequestHandler::CGI::Env::clear_c_env( char **envp ) {
    for ( char **p( envp ); *p; p++ ) { delete[] * p; }
    delete[] envp;
}

/* ----------------------------- RequestHandler ----------------------------- */

RequestHandler::RequestHandler( Ptr::Shared< HTTP::Request > request,
                                Ptr::Shared< ServerConf >    conf )
    : _request( request ),
      _conf( conf ),
      _route( 0 ) {
    if ( _conf->route_mapper().count( _request->url() + '/' ) ) {
        _route = &_conf->route_mapper().at( _request->url() + '/' );
        _path  = _route->root() + '/'
                + _conf->route_mapper().suffix( _request->url() );
    }
}

HTTP::Response RequestHandler::make_response() {
    if ( !_route ) {
        return HTTP::Response::make_error_response(
            HTTP::Response::E404,
            _conf->code_to_error_page() );
    }
    if ( !_route->methods().count( _request->method() ) ) {
        return HTTP::Response::make_error_response(
            HTTP::Response::E405,
            _conf->code_to_error_page() );
    }
    for ( std::map< std::string, std::string >::const_iterator it
          = _route->cgis().begin();
          it != _route->cgis().end();
          it++ ) {
        std::string cgi_path
            = _route->handler().size() ? _route->handler() : _path;
        if ( Str::ends_with( cgi_path, it->first ) ) {
            return _cgi( it->second, cgi_path );
        }
    }
    if ( _route->redir().size() ) { return _redir(); }
    switch ( HTTP::Request::method_to_string().at( _request->method() ) ) {
    case HTTP::Request::GET: return _get();
    case HTTP::Request::POST: return _post();
    case HTTP::Request::DELETE: return _delete();
    }
}

HTTP::Response RequestHandler::_get() {
    struct stat s;
    if ( stat( _path.c_str(), &s ) ) {
        return errno == ENOENT ? HTTP::Response::make_error_response(
                   HTTP::Response::E404,
                   _conf->code_to_error_page() )
                               : HTTP::Response::make_error_response(
                                   HTTP::Response::E500,
                                   _conf->code_to_error_page() );
    }
    if ( s.st_mode & S_IFDIR ) {
        if ( _route->index().size() ) {
            std::ifstream f( ( _path + '/' + _route->index() ).c_str() );
            if ( f.is_open() ) {
                _path += '/' + _route->index();
                HTTP::Response r( HTTP::Response::E200 );
                r.set_content(
                    std::string( ( std::istreambuf_iterator< char >( f ) ),
                                 std::istreambuf_iterator< char >() ) );
                r.header["Content-Type"] = _content_type( _route->index() );
                return r;
            }
        }

        if ( _route->autoindex() ) { return _autoindex(); }
        return HTTP::Response::make_error_response(
            HTTP::Response::E404,
            _conf->code_to_error_page() );
    }
    std::ifstream f( _path.c_str() );
    if ( !f.is_open() ) {
        return HTTP::Response::make_error_response(
            HTTP::Response::E404,
            _conf->code_to_error_page() );
    }
    HTTP::Response r( HTTP::Response::E200 );
    r.set_content( std::string( ( std::istreambuf_iterator< char >( f ) ),
                                std::istreambuf_iterator< char >() ) );
    r.header["Content-Type"] = _content_type( _path );
    return r;
}

HTTP::Response RequestHandler::_post() {
    return HTTP::Response( HTTP::Response::E500 );
}

HTTP::Response RequestHandler::_delete() {
    struct stat s;
    if ( stat( _path.c_str(), &s ) == 0 ) {
        if ( s.st_mode & S_IFDIR ) {
            return HTTP::Response( HTTP::Response::E400 );
        } else {
            std::remove( _path.c_str() );
            return HTTP::Response( HTTP::Response::E200 );
        }
    }
    return errno == ENOENT ? HTTP::Response( HTTP::Response::E404 )
                           : HTTP::Response( HTTP::Response::E500 );
}

HTTP::Response RequestHandler::_autoindex() {
    struct dirent *file;
    std::string    content    = "<!DOCTYPE html><html><body><h1>";
    std::string    contentEnd = "</h1></body></html>";
    DIR           *dir;
    dir = opendir( _path.c_str() );
    if ( !dir )
        return HTTP::Response::make_error_response(
            HTTP::Response::E500,
            _conf->code_to_error_page() );
    while ( ( file = readdir( dir ) ) != NULL ) {
        content += std::string( "<p><a href='" )
                   + _conf->route_mapper().route_name( _request->url() )
                   + _conf->route_mapper().suffix( _request->url() );
        if ( !Str::ends_with( content, "/" ) ) { content += '/'; }
        content
            += std::string( file->d_name ) + "'>" + file->d_name + "</a></p>";
    }
    content += contentEnd;
    closedir( dir );
    HTTP::Response r( HTTP::Response::E200 );
    r.set_content( content );
    r.header["Content-Type"] = _content_type( ".html" );
    return r;
}

HTTP::Response RequestHandler::_redir() {
    HTTP::Response r( HTTP::Response::E301 );
    r.header["Location"] = _route->redir();
    return r;
}

HTTP::Response RequestHandler::_cgi( const std::string &bin_path,
                                     const std::string &path ) {
    std::string bp( bin_path );
    std::string p( path );
    char       *args[] = { const_cast< char       *>( bp.c_str() ),
                           const_cast< char       *>( p.c_str() ),
                           0 };
    CGI::Env    env;
    env[CGI::PATH_INFO] = path;
    env[CGI::REQUEST_METHOD]
        = HTTP::Request::method_to_string().at( _request->method() );
    if ( _request->header().count( "Content-Type" ) ) {
        env[CGI::CONTENT_TYPE] = _request->header().at( "Content-Type" );
    }
    if ( _request->content().operator->() ) {
        env[CGI::CONTENT_LENGTH] = Str::from( _request->content()->size() );
    } else {
        env[CGI::CONTENT_LENGTH] = "0";
    }
    env[CGI::QUERY_STRING]    = _route->root();
    env[CGI::REDIRECT_STATUS] = "200";
    env[CGI::SCRIPT_FILENAME] = path;
    if ( _request->header().count(
             HTTP::Request::key_to_string().at( HTTP::Request::COOKIE ) ) ) {
        env[CGI::HTTP_COOKIE] = _request->header().at(
            HTTP::Request::key_to_string().at( HTTP::Request::COOKIE ) );
    }
    int i_pipe[2];
    int o_pipe[2];
    if ( ::pipe( i_pipe ) == -1 ) { throw std::runtime_error( "pipe" ); }
    if ( ::pipe( o_pipe ) == -1 ) {
        close( i_pipe[0] );
        close( i_pipe[1] );
        throw std::runtime_error( "pipe" );
    }
    int pid = ::fork();
    if ( pid == -1 ) { throw std::runtime_error( "fork" ); }
    if ( !pid ) {
        char **envp( env.c_arr() );
        if ( ::dup2( i_pipe[0], STDIN_FILENO ) == -1 ) {
            throw std::runtime_error( "dup2" );
        }
        ::close( i_pipe[1] );
        if ( ::dup2( o_pipe[1], STDOUT_FILENO ) == -1 ) {
            throw std::runtime_error( "pipe" );
        }
        ::close( o_pipe[0] );
        ::execve( *args, args, envp );
        CGI::Env::clear_c_env( envp );
        ::exit( EXIT_FAILURE );
    }
    close( i_pipe[0] );
    close( o_pipe[1] );
    if ( _request->content().operator->()
         && write( i_pipe[1],
                   _request->content()->c_str(),
                   _request->content()->size() )
                == -1 ) {
        throw std::runtime_error( "write" );
    }
    close( i_pipe[1] );
    char        buff[1024];
    size_t      n;
    std::string s;
    int         exit_code;
    wait( &exit_code );
    if ( !WIFEXITED( exit_code ) | WEXITSTATUS( exit_code ) ) {
        return HTTP::Response::make_error_response(
            HTTP::Response::E500,
            _conf->code_to_error_page() );
    }
    while ( ( n = ::read( o_pipe[0], buff, 1024 ) ) == 1024 ) {
        s.append( buff, n );
    }
    ::close( o_pipe[0] );
    s.append( buff, n );
    HTTP::Response r( HTTP::Response::E200 );
    std::string    raw_header
        = s.substr( 0, std::min( s.find( "\n\n" ), s.find( "\r\n\r\n" ) ) );
    r.header = HTTP::Header::from_string( raw_header ).unwrap();
    r.set_content(
        s.substr( std::min( s.find( "\n\n" ), s.find( "\r\n\r\n" ) ) ) );
    return r;
}

const std::string &
RequestHandler::_content_type( const std::string &path ) const {
    static const std::string default_type( "text/plain" );
    size_t                   found( path.find_last_of( '.' ) );
    if ( found == std::string::npos ) return ( default_type );
    std::string last( path, found + 1 );
    if ( !HTTP::Mime::extension_to_type().count( last ) ) {
        return default_type;
    }
    return HTTP::Mime::extension_to_type().at( last );
}

/* -------------------------------------------------------------------------- */
