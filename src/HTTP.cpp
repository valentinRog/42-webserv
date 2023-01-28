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

const std::map< std::string, std::string > &HTTP::Values::extension_to_mime() {
    struct f {
        static std::map< std::string, std::string > init() {
            std::map< std::string, std::string > m;
            JSON::Object o( JSON::Parse::from_file( "mime.json" )
                                .unwrap< JSON::Object >() );
            for ( JSON::Object::const_iterator it( o.begin() ); it != o.end();
                  it++ ) {
                JSON::Array a( it->second.unwrap< JSON::Array >() );
                for ( JSON::Array::const_iterator nit( a.begin() );
                      nit != a.end();
                      nit++ ) {
                    m[nit->unwrap< JSON::String >()] = it->first;
                }
            }
            return m;
        }
    };
    static const std::map< std::string, std::string > m( f::init() );
    return m;
}

/* -------------------------------------------------------------------------- */

HTTP::DynamicParser::DynamicParser()
    : _step( REQUEST ),
      _request( new Request() ) {}

void HTTP::DynamicParser::operator<<( const std::string &s ) {
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

HTTP::DynamicParser::e_step HTTP::DynamicParser::step() const { return _step; }

Ptr::shared< HTTP::Request > HTTP::DynamicParser::request() { return _request; }

void HTTP::DynamicParser::_parse_line() {
    std::istringstream iss( _line );
    switch ( _step ) {
    case REQUEST:
        iss >> _request->method;
        iss >> _request->url;
        _request->url += '/';
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

HTTP::RequestHandler::RequestHandler( Ptr::shared< Request >    request,
                                      Ptr::shared< ServerConf > conf )
    : _request( request ),
      _conf( conf ),
      _route( _conf->routes_table.at(
          _conf->routes.lower_bound( _request->url) ) ),
      _contentType( getContentType( _route.path ) ) {
    response();
}

void HTTP::RequestHandler::response() {
    if ( _request->version != "HTTP/1.1" ) {
        setResponse( 505, errorMessage( 505 ) );
        return;
    }
    if ( _route.redir != "" )
        toRedir();
    else if ( _request->method == "GET" && _route.methods.count( "GET" ) )
        getMethod();
    else if ( _request->method == "POST" && _route.methods.count( "POST" ) )
        postMethod();
    else if ( _request->method == "DELETE" && _route.methods.count( "DELETE" ) )
        deleteMethod();
    else { setResponse( 405, errorMessage( 405 ) ); }
}

void HTTP::RequestHandler::getMethod() {
    struct stat s;
    std::cout << "path = " << _get_path() << std::endl;
    if ( stat( _get_path().c_str(), &s ) == 0 ) {
        if ( s.st_mode & S_IFDIR
             && ( _route.root != _get_path() || _route.autoindex ) ) {
            if ( _route.autoindex )
                toDirListing();
            else
                setResponse( 404, errorMessage( 404 ) );
        } else {
            // std::string path;
            // for ( std::list< std::string >::const_iterator it
            //       = _route.index.begin();
            //       it != _route.index.end();
            //       it++ ) {
            //     // std::cout << _route.root + *it << std::endl;
            //     if ( stat( ( _route.root + *it ).c_str(), &s ) == 0 ) {
            //         path         = *it;
            //         _contentType = getContentType( path );
            //         break;
            //     }
            // }
            // if ( path.empty() ) setResponse( 404, errorMessage( 404 ) );
            std::ifstream      fd( ( _get_path() ).c_str() );
            std::ostringstream oss;
            oss << fd.rdbuf();
            std::string page( oss.str() );
            setResponse( 200, page );
        }
    } else {
        if ( errno == ENOENT )
            setResponse( 404, errorMessage( 404 ) );
        else { setResponse( 500, errorMessage( 500 ) ); }
    }
}

void HTTP::RequestHandler::postMethod() {}

void HTTP::RequestHandler::deleteMethod() {
    struct stat s;
    if ( stat( _get_path().c_str(), &s ) == 0 ) {
        if ( s.st_mode & S_IFDIR ) {
            setResponse( 400, errorMessage( 400 ) );
        } else {
            std::remove( _get_path().c_str() );
            setResponse( 200, "" );
        }
    } else {
        if ( errno == ENOENT )
            setResponse( 404, errorMessage( 404 ) );
        else { setResponse( 500, errorMessage( 500 ) ); }
    }
}

void HTTP::RequestHandler::toDirListing() {
    struct dirent *file;
    std::string    content    = "<!DOCTYPE html><html><body><h1>";
    std::string    contentEnd = "</h1></body></html>";

    DIR *dir;
    dir = opendir( _get_path().c_str() );
    if ( !dir ) return;
    while ( ( file = readdir( dir ) ) != NULL )
        content += "<p>" + std::string( file->d_name ) + "</p>";
    content += contentEnd;
    closedir( dir );
    _contentType = getContentType( ".html" );
    setResponse( 200, content );
}

void HTTP::RequestHandler::toRedir() {
    std::string content = "<meta http-equiv=\"refresh\" content=\"0;URL="
                          + _route.redir + "\"/>";
    _contentType = getContentType( ".html" );
    setResponse( 301, content );
}

void HTTP::RequestHandler::setResponse( int nb, std::string content ) {
    _response.code    = JSON::Number( nb ).stringify();
    _response.outcome = HTTP::Values::error_code_to_message().at( nb );
    _response.version = _request->version;
    _response.content = content;

    if ( _route.redir.empty() )
        _response.header["Content-Type"] = _contentType;
    else { _response.header["Location"] = _route.redir; }
    _response.header["Host"] = "ddfdfdfd";
    if ( !content.empty() ) {
        std::stringstream ss;
        ss << content.size();
        _response.header["Content-Length"] = ss.str();
    }
    std::cout << _response.stringify();
}

std::string HTTP::RequestHandler::errorMessage( int nb ) {
    std::string content;

    content = "<!DOCTYPE html><html>";
    content += "<h1>" + JSON::Number( nb ).stringify() + "</h>";
    content += "<p>" + HTTP::Values::error_code_to_message().at( nb )
               + "</p></html>";
    _contentType = getContentType( ".html" );
    return ( content );
}

HTTP::Response HTTP::RequestHandler::getResponse() { return ( _response ); }

std::string HTTP::RequestHandler::getContentType( std::string path ) {
    size_t found = path.find_last_of( '.' );
    if ( found == std::string::npos ) return ( "text/plain" );
    std::string last( path, found );
    if ( last.size() ) { last = last.substr( 1, last.size() - 1 ); }
    if ( !Values::extension_to_mime().count( last ) ) { return "text/plain"; }
    return Values::extension_to_mime().at( last );
}

/* -------------------------------------------------------------------------- */
