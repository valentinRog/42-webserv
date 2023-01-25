#include "HTTP.hpp"

/* -------------------------------------------------------------------------- */

std::ostream &operator<<( std::ostream &os, const HTTP::Request &r ) {
    JSON::Object o;
    o.insert( std::make_pair( "method", JSON::String( r.method ) ) );
    o.insert( std::make_pair( "url", JSON::String( r.url ) ) );
    o.insert( std::make_pair( "version", JSON::String( r.version ) ) );
    o.insert( std::make_pair( "host", JSON::String( r.host ) ) );
    o.insert( std::make_pair( "header", JSON::Object() ) );
    JSON::Object o2( o.at( "header" ).unwrap< JSON::Object >() );
    for ( std::map< std::string, std::string >::const_iterator it(
              r.header.begin() );
          it != r.header.end();
          it++ ) {
        o2.insert( std::make_pair( it->first, JSON::String( it->second ) ) );
    }
    o.at( "header" ) = o2;
    return os << o.stringify();
}

/* -------------------------------------------------------------------------- */

HTTP::DynamicParser::DynamicParser() : _step( REQUEST ) {}

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

const HTTP::Request &HTTP::DynamicParser::request() const { return _request; }

void HTTP::DynamicParser::_parse_line() {
    std::istringstream iss( _line );
    switch ( _step ) {
    case REQUEST:
        iss >> _request.method;
        iss >> _request.url;
        iss >> _request.version;
        _step = HOST;
        break;
    case HOST:
        iss.ignore( std::numeric_limits< std::streamsize >::max(), ' ' );
        iss >> _request.host;
        _step = HEADER;
        break;
    case HEADER: {
        if ( !iss.str().size() ) {
            _step = CONTENT;
        } else {
            std::string k;
            std::string v;
            iss >> k >> v;
            k                  = k.substr( 0, k.size() - 1 );
            _request.header[k] = v;
            break;
        }
    }
    case CONTENT: _step = DONE; break;
    }
}

/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */

HTTP::RequestHandler::RequestHandler( const Request &          request,
                                      const VirtualHostMapper &vhm )
    : _request( request ),
      _conf( vhm[request.host] ),
      _route(
          _conf.routes_table.at( _conf.routes.lower_bound( _request.url ) ) ),
      _contentType(getContentType(_route.path)) {
    response();
}

void HTTP::RequestHandler::response() {
    

    if ( _route.redir != "" )
        toRedir();
    else if ( _request.method == "GET" && _route.methods.count( "GET" ) )
        getMethod();
    else if ( _request.method == "POST" && _route.methods.count( "POST" ) )
        postMethod();
    else if ( _request.method == "DELETE" && _route.methods.count( "DELETE" ) )
        deleteMethod();
    else {
        setResponse(403, "");
    }
}

void HTTP::RequestHandler::getMethod() {
    struct stat s;
    if (stat((getenv("PWD") + _route.root + _route.path).c_str(), &s) == 0) {
        if (s.st_mode & S_IFDIR && !_route.path.empty()) {
            if (_route.autoindex == true)
                toDirListing();
            else {
                setResponse(404, "");
            }
        } else {
            std::string path;
            if (_route.path.empty()) {
                for (std::list<std::string>::const_iterator it = _route.index.begin(); it !=_route.index.end(); it++) {
                    if (stat((getenv("PWD") + _route.root + *it).c_str(), &s) == 0) {
                        path = *it;
                        _contentType = getContentType(path);
                        break ;
                    }
                }
                if (path.empty())
                    setResponse(404, "");
            }
            else
                path = _route.path;         
            std::ifstream fd((getenv("PWD") + _route.root + path).c_str());
            std::string page( ( std::istreambuf_iterator< char >( fd ) ), std::istreambuf_iterator< char >());
            setResponse(200, page);
        }
    } else {
        if (errno == ENOENT)
            setResponse(404, "");
        else
            setResponse(400, "");
    }
}

void HTTP::RequestHandler::postMethod() {}

void HTTP::RequestHandler::deleteMethod() {
    struct stat s;

    if (stat((getenv("PWD") + _route.root + _route.path).c_str(), &s) == 0) {
        if (s.st_mode & S_IFDIR) {
            setResponse(400, "");
        } else {
            std::remove((getenv("PWD") + _route.root + _route.path).c_str());
            setResponse(200, "");
        }
    } else {
        if (errno == ENOENT)
            setResponse(404, "");
        else
            setResponse(400, "");
    }
}

void HTTP::RequestHandler::toDirListing() {
    struct dirent *file;
    std::string content = _request.version + "<!DOCTYPE html><html><body><h1>";
    std::string contentEnd = "</h1></body></html>";

    DIR *dir;
    dir = opendir( ( getenv("PWD") + _route.root + _route.path ).c_str() );
    if ( !dir ) return;
    while ( ( file = readdir( dir ) ) != NULL )
        content += "<p>" + std::string( file->d_name ) + "</p>";
    content += contentEnd;
    closedir(dir);
    setResponse(200, content);
}

void HTTP::RequestHandler::toRedir() {
    std::string content = "<head><meta http-equiv=\"Refresh\" content=\"0;url="
                            + _route.redir + "\"/></head>";
    setResponse(301, content);
}

void HTTP::RequestHandler::setResponse( int nb, std::string content) {
    std::ostringstream ss;
    ss << nb;
    _response.code = ss.str();
    _response.outcome = "OK"; //le truc static ?
    _response.version = _request.version;
    _response.content = content;

    if (_route.redir.empty())
        _response.header.insert(std::pair<std::string, std::string>("content-Type:", _contentType));
    else
        _response.header.insert(std::pair<std::string, std::string>("Location:", _route.redir));
    _response.header.insert(std::pair<std::string, std::string>("Host:", "?????")); //quemettre
    if (!content.empty()) {
        std::stringstream ss;
        ss << content.size();
        _response.header.insert(std::pair<std::string, std::string>("Content-Length:", ss.str()));
    }
    std::cout << _response.stringify();
}

HTTP::Response HTTP::RequestHandler::getResponse() {
    return (_response);
}

std::string HTTP::RequestHandler::getContentType( std::string path ) {
    size_t found = path.find_last_of( '.' );

    if ( found == std::string::npos ) return ( "text/plain" );
    std::string last( path, found );
    if ( last == ".html" ) {
        return ( "text/html" );
    } else if ( last == ".css" ) {
        return ( "text/css" );
    } else if ( last == ".csv" ) {
        return ( "text/csv" );
    } else if ( last == ".xml" ) {
        return ( "text/xml" );
    } else if ( last == ".gif" ) {
        return ( "image/gif" );
    } else if ( last == ".jpeg" ) {
        return ( "image/jpeg" );
    } else if ( last == ".png" ) {
        return ( "image/png" );
    } else if ( last == ".tiff" ) {
        return ( "image/tiff" );
    } else if ( last == ".javascript" ) {
        return ( "application/javascript" );
    } else if ( last == ".pdf" ) {
        return ( "application/pdf" );
    } else if ( last == ".json" ) {
        return ( "application/json" );
    } else {
        return ( "text/plain" );
    }
}

/* -------------------------------------------------------------------------- */
