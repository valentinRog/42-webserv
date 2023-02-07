#include "RequestHandler.hpp"

/* --------------------------- RequestHandler::CGI -------------------------- */

const std::string &
RequestHandler::CGI::env_key_to_string( RequestHandler::CGI::e_env_key key ) {
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

char **RequestHandler::CGI::Env::c_arr() const {
    char **env = new char *[size() + 1];
    size_t i( 0 );
    for ( const_iterator it( begin() ); it != end(); it++, i++ ) {
        env[i]
            = Str::dup( ( env_key_to_string( it->first ) ) + "=" + it->second );
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

HTTP::Response
RequestHandler::make_error_response( HTTP::Response::e_error_code code,
                                     const ServerConf            *conf ) {
    HTTP::Response r( code );
    std::string    content;
    if ( conf && conf->code_to_error_page().count( code ) ) {
        std::string page( conf->code_to_error_page().at(code ));
        std::ostringstream oss;
        std::ifstream      f( page.c_str() );
        oss << f.rdbuf();
        content = oss.str();
        r.set_content( content );
    } else {
        r.set_content( "<h1>"
                       + HTTP::Response::error_code_to_string( code ).first
                       + "</h1>" );
    }
    return r;
}

std::string RequestHandler::make_raw_response() {
    if ( !_route ) {
        return make_error_response( HTTP::Response::E404, _conf.operator->() )
            .stringify();
    }
    if ( !_route->methods().count(
             HTTP::Request::method_to_string( _request->method() ) ) ) {
        return make_error_response( HTTP::Response::E405, _conf.operator->() )
            .stringify();
    }
    for ( std::map< std::string, std::string >::const_iterator it
          = _route->cgis().begin();
          it != _route->cgis().end();
          it++ ) {
        if ( Str::ends_with( _path, it->first ) ) { return _cgi( it->second ); }
    }
    if ( _route->redir().size() ) { return _redir().stringify(); }
    switch ( _request->method() ) {
    case HTTP::Request::GET: return _get().stringify();
    case HTTP::Request::POST: return _post().stringify();
    case HTTP::Request::DELETE: return _delete().stringify();
    }
}

HTTP::Response RequestHandler::_get() {
    struct stat s;
    if ( stat( _path.c_str(), &s ) ) {
        return errno == ENOENT ? make_error_response( HTTP::Response::E404,
                                                      _conf.operator->() )
                               : make_error_response( HTTP::Response::E500,
                                                      _conf.operator->() );
    }
    if ( s.st_mode & S_IFDIR ) {
        for ( std::list< std::string >::const_iterator it(
                  _route->index().begin() );
              it != _route->index().end();
              it++ ) {
            std::ifstream f( ( _path + '/' + *it ).c_str() );
            if ( f.is_open() ) {
                _path += '/' + *it;
                HTTP::Response r( HTTP::Response::E200 );
                r.set_content(
                    std::string( ( std::istreambuf_iterator< char >( f ) ),
                                 std::istreambuf_iterator< char >() ) );
                r.header[HTTP::Response::CONTENT_TYPE] = _content_type( *it );
                return r;
            }
        }
        if ( _route->autoindex() ) { return _autoindex(); }
        return make_error_response( HTTP::Response::E404, _conf.operator->() );
    }
    std::ifstream f( _path.c_str() );
    if ( !f.is_open() ) {
        return make_error_response( HTTP::Response::E404, _conf.operator->() );
    }
    HTTP::Response r( HTTP::Response::E200 );
    r.set_content( std::string( ( std::istreambuf_iterator< char >( f ) ),
                                std::istreambuf_iterator< char >() ) );
    r.header[HTTP::Response::CONTENT_TYPE] = _content_type( _path );
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
        return make_error_response( HTTP::Response::E500, _conf.operator->() );
    while ( ( file = readdir( dir ) ) != NULL ) {
        content += std::string( "<p><a href='" )
                   + _conf->route_mapper().route_name( _request->url() ) + '/'
                   + _conf->route_mapper().suffix( _request->url() ) + '/'
                   + file->d_name + "'>" + file->d_name + "</a></p>";
    }
    content += contentEnd;
    closedir( dir );
    HTTP::Response r( HTTP::Response::E200 );
    r.set_content( content );
    r.header[HTTP::Response::CONTENT_TYPE] = _content_type( ".html" );
    return r;
}

HTTP::Response RequestHandler::_redir() {
    HTTP::Response r( HTTP::Response::E301 );
    r.header[HTTP::Response::LOCATION] = _route->redir();
    return r;
}

std::string RequestHandler::_cgi( const std::string &bin_path ) {
    std::string bp( bin_path );
    std::string p( _path );
    char       *args[] = { const_cast< char       *>( bp.c_str() ),
                           const_cast< char       *>( p.c_str() ),
                           0 };
    CGI::Env    env;
    env[CGI::PATH_INFO] = _path;
    env[CGI::REQUEST_METHOD]
        = HTTP::Request::method_to_string( _request->method() );
    if ( _request->header().count( "Content-Type" ) ) {
        env[CGI::CONTENT_TYPE] = _request->header().at( "Content-Type" );
    }
    env[CGI::CONTENT_LENGTH]  = Str::from( _request->content().size() );
    env[CGI::QUERY_STRING]    = "";
    env[CGI::REDIRECT_STATUS] = "200";
    env[CGI::SCRIPT_FILENAME] = _path;
    int i_pipe[2];
    int o_pipe[2];
    if ( ::pipe( i_pipe ) == -1 ) {
        return RequestHandler::make_error_response( HTTP::Response::E500,
                                                    _conf.operator->() )
            .stringify();
    }
    if ( ::pipe( o_pipe ) == -1 ) {
        close( i_pipe[0] );
        close( i_pipe[1] );
    }
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
    return Str::trim_right( HTTP::Response( HTTP::Response::E200 ).stringify(),
                            "\r\n" )
           + "\r\n" + s;
}

const std::string &
RequestHandler::_content_type( const std::string &path ) const {
    static const std::string default_type( "text/plain" );
    size_t                   found( path.find_last_of( '.' ) );
    if ( found == std::string::npos ) return ( default_type );
    std::string last( path, found + 1 );
    if ( !_conf->mime().count( last ) ) { return default_type; }
    return _conf->mime().at( last );
}

/* -------------------------------------------------------------------------- */
