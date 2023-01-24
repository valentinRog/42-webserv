#include "HTTP.hpp"
#include <sys/stat.h>
#include <unistd.h>

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
    return o.repr( os );
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
            iss >> _request.content;
            break;
        }
        std::string k;
        std::string v;
        iss >> k >> v;
        k                  = k.substr( 0, k.size() - 1 );
        _request.header[k] = v;
        break;
    }
    case CONTENT: break;
    }
}

HTTP::Request HTTP::DynamicParser::getRequest() { return (_request); }

/* -------------------------------------------------------------------------- */

HTTP::Response::Response() {
    _responseStatus[200] = "OK";
    _responseStatus[201] = "Created";
    _responseStatus[400] = "Bad Request";
    _responseStatus[403] = "Forbidden";
    _responseStatus[404] = "Not Found";
    _responseStatus[405] = "Method Not Allowed";
    _responseStatus[505] = "Version Not Supported";
}

std::string HTTP::Response::response( Request  httpRequest, const ServerConf &serv ) {
    //verif HTTP et method qui existe 
    std::map<std::string, ServerConf::Route>::const_iterator it;  
    it = serv.routes_table.find(httpRequest.url);
    if (it != serv.routes_table.end()) {
        const ServerConf::Route route = it->second;

        if (route.redir != "")
            toRedir(httpRequest, route);
        else if ( httpRequest.method == "GET" && route.methods.count( "GET" ) )
            getMethod(httpRequest, route);
        else if ( httpRequest.method == "POST" && route.methods.count( "POST" ) )
            postMethod();
        else if ( httpRequest.method == "DELETE" && route.methods.count( "DELETE" ) )
            deleteMethod(httpRequest, route);
        else
            setResponse( 403, _defaultPathError, httpRequest); //+ error dif si method mal écrite
    } else {
        //ERROR mauvais path 403 j pense;
        return (NULL);
    }
    return (_res);
}

void HTTP::Response::getMethod(Request httpRequest, const ServerConf::Route & route) {
    struct stat s;

    if (stat((_rootPath + route.root + route.path).c_str(), &s) == 0) {
        if (s.st_mode & S_IFDIR) {
            if (route.autoindex == true)
                toDirListing(httpRequest, route);
            else {
                //error 404 is a directory
            }
        } else {
            //go send
        }
    } else {
        //ENOENT: Le fichier ou répertoire spécifié n'existe pas.
    }
}

void HTTP::Response::postMethod() {}

void HTTP::Response::deleteMethod(Request httpRequest, const ServerConf::Route & route) {
    struct stat s;

    if (stat((_rootPath + route.root + route.path).c_str(), &s) == 0) {
        if (s.st_mode & S_IFDIR) {
            //error is a direectory 403
        } else {
            std::remove((_rootPath + route.root + route.path).c_str());
            //go send
        }
    } else {
        //ERROR
    }
}

void HTTP::Response::toDirListing(Request httpRequest, const ServerConf::Route & route) {
    struct dirent *file;
    std::string res = httpRequest.version + " 200 OK\n\n<!DOCTYPE html><html><body><h1>";
    std::string resEnd = "</h1></body></html>";

    DIR *dir;
    dir = opendir( ( _rootPath + route.root + route.path ).c_str() );
    if ( !dir ) return;
    while ( ( file = readdir( dir ) ) != NULL )
        res += "<p>" + std::string( file->d_name ) + "</p>";
    res += resEnd;
    closedir(dir);
    std::cout << "Dir listing finish" << std::endl;
}

void HTTP::Response::toRedir(Request httpRequest, const ServerConf::Route & route) {
    std::string res
        = httpRequest.version
        + " 200 OK\n\n<head><meta http-equiv=\"Refresh\" content=\"0;url="
        + route.redir + "\"/></head>";
}

void HTTP::Response::setResponse( int nb, std::string rPath, Request httpRequest) {
    std::ifstream fd((_rootPath + rPath).c_str());
    std::string        page( ( std::istreambuf_iterator< char >( fd ) ), std::istreambuf_iterator< char >() );
    std::ostringstream ss;
    ss << nb;
    _res = httpRequest.version + " " + ss.str()
            + " " + _responseStatus.find(nb)->second + "\r\n"
            + "Content-type:" + getContentType( rPath ) + "\r\n";
    ss.clear();
    ss << ( page ).size();
    _res = _res + "Content-length:" + ss.str() + "\r\n\r\n";
    _res = _res + page + "\r\n";
    std::cout << _res << std::endl;
}

std::string HTTP::Response::getContentType( std::string path ) {
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
    } else if ( last == ".javascript" ) {
        return ( "application/javascript" );
    } else {
        return ( "text/plain" );
    }
}

/* -------------------------------------------------------------------------- */
