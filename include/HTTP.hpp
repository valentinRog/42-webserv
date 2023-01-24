#pragma once

#include "JSON.hpp"
#include "VirtualHostMapper.hpp"
#include "common.h"

namespace HTTP {

/* -------------------------------------------------------------------------- */

struct Request {
    std::string                          method;
    std::string                          url;
    std::string                          version;
    std::string                          host;
    std::map< std::string, std::string > header;
};

std::ostream &operator<<( std::ostream &os, const Request &r );

/* -------------------------------------------------------------------------- */

class DynamicParser {
public:
    enum e_step { REQUEST, HOST, HEADER, CONTENT, DONE };

private:
    std::string _line;
    std::string _sep;
    e_step      _step;
    Request     _request;

public:
    DynamicParser();

    void           operator<<( const std::string &s );
    e_step         step() const;
    const Request &request() const;

private:
    void _parse_line();
};

/* -------------------------------------------------------------------------- */

struct Response {
    std::string                          version;
    std::string                          code;
    std::string                          outcome;
    std::map< std::string, std::string > header;
    std::string                          content;

    Response()
        : version( "HTTP/1.1" ),
          code( "200" ),
          outcome( "OK" ),
          content( "yo\n" ) {
        header["Content-Type"] = "text/html";
        std::ostringstream oss;
        oss << content.size();
        header["Content-Length"] = oss.str();
    }

    operator std::string() const {
        std::string s( version + ' ' + code + ' ' + outcome + "\r\n" );
        for ( std::map< std::string, std::string >::const_iterator it
              = header.begin();
              it != header.end();
              it++ ) {
            s += it->first + ": " + it->second + "\r\n";
        }
        return s + "\r\n" + content;
    }
};

/* -------------------------------------------------------------------------- */

class RequestHandler {
    std::map< int, std::string > _responseStatus;

    Request    _request;
    ServerConf _conf;
    Response   _response;

    const ServerConf::Route &_route;

    // std::string _route;
    // std::string _root;
    std::string _path;

    // const ServerConf::Route &_route;

    // //request
    // std::string _methodRequest;
    // std::string _version;
    // std::string _rootPath;

    // //serv
    // std::string             _path;
    // std::string             _root;
    // std::set< std::string > _allowedMethod;
    // std::string             _defaultPathError;
    // std::string             _index;
    // bool                    _dirListing;
    // std::string             _redir;

    // ServerConf _conf;

public:
    RequestHandler( const Request &request, const VirtualHostMapper &vhm );

    // void
    //      setInformation( Request httpRequest, int clientFd, const ServerConf &serv );
    // void response( Request httpRequest, int clientFd, const ServerConf &serv );
    // int  verifLocation( std::string                        path,
    //                     std::vector< ServerConf::Route * > locs );

    // void toRedir();
    // void toDirListing();
    void getMethod();
    // void postMethod();
    // void deleteMethod();

    const Response &response() const { return _response; }

    // std::string getContentType( std::string path );
    // void        sendResponse( int nb, std::string page );
private:
    std::string _get_path() const {
        std::string route( _conf.routes.lower_bound( _request.url ) );
        std::string url( _request.url );
        return _route.root
               + url.substr( route.size(), url.size() - route.size() );
    }
};

/* -------------------------------------------------------------------------- */

}
