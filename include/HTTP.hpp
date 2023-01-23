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
    enum step { REQUEST, HOST, HEADER, CONTENT };

    std::string _line;
    std::string _sep;
    step        _step;
    Request     _request;

public:
    DynamicParser();

    void operator<<( const std::string &s );

private:
    void _parse_line();
};

/* -------------------------------------------------------------------------- */

class Response {
    std::map< int, std::string > _responseStatus;

    //request
    int         _clientFd;
    std::string _methodRequest;
    std::string _version;
    std::string _rootPath;

    //serv
    std::string             _path;
    std::string             _root;
    std::set< std::string > _allowedMethod;
    std::string             _defaultPathError;
    std::string             _index;
    bool                    _dirListing;
    std::string             _redir;

    ServerConf _conf;

public:
    Response( const Request &request );
    void setInformation( Request       httpRequest,
                         int               clientFd,
                         const ServerConf &serv );
    void
    response( Request httpRequest, int clientFd, const ServerConf &serv );
    int verifLocation( std::string                           path,
                       std::vector< ServerConf::Route * > locs );

    void toRedir();
    void toDirListing();
    void getMethod();
    void postMethod();
    void deleteMethod();

    std::string getContentType( std::string path );
    void        sendResponse( int nb, std::string page );
};


/* -------------------------------------------------------------------------- */

}