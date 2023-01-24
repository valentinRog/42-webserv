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
    std::string                          content;
    std::map< std::string, std::string > header;
};

std::ostream &operator<<( std::ostream &os, const Request &r );

/* -------------------------------------------------------------------------- */

class DynamicParser {
    enum step { REQUEST, HOST, HEADER, CONTENT };

    std::string _line;
    std::string _sep;
    step        _step;
    Request     _request;

public:
    DynamicParser();
    Request getRequest();

    void operator<<( const std::string &s );

private:
    void _parse_line();
};

/* -------------------------------------------------------------------------- */

class Response {
    std::map< int, std::string > _responseStatus;

    //request
    std::string _methodRequest;
    std::string _version;
    std::string _rootPath;

    std::string _defaultPathError;

    std::string _res;

public:
    Response();
    std::string response( Request httpRequest, const ServerConf &serv );

    void toRedir(Request httpRequest, const ServerConf::Route & route);
    void toDirListing(Request httpRequest, const ServerConf::Route & route);
    void getMethod(Request httpRequest, const ServerConf::Route & route);
    void postMethod();
    void deleteMethod(Request httpRequest, const ServerConf::Route & route);

    std::string getContentType( std::string path );
    void        setResponse( int nb, std::string page, Request httpRequest);
};


/* -------------------------------------------------------------------------- */

}
