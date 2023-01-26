#pragma once

#include "JSON.hpp"
#include "VirtualHostMapper.hpp"
#include "common.h"

namespace HTTP {

struct ErrorCodes {
    static const std::map<int, std::string> messages;

  private:
    static std::map<int, std::string> _init_messages() {
    std::map<int, std::string> m;
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

struct Response : public Trait::Stringify {
    std::string                          version;
    std::string                          code;
    std::string                          outcome;
    std::map< std::string, std::string > header;
    std::string                          content;

    std::string stringify() const {
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
    std::string _contentType;

public:
    RequestHandler( const Request &request, const VirtualHostMapper &vhm );
    void response();
    void toRedir();
    void toDirListing();
    void getMethod();
    void postMethod();
    void deleteMethod();

    std::string errorMessage(int nb);
    HTTP::Response getResponse();
    std::string getContentType( std::string path );
    void        setResponse( int nb, std::string content);
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
