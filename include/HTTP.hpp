#pragma once

#include "JSON.hpp"
#include "Ptr.hpp"
#include "common.h"
#include "ServerConf.hpp"

namespace HTTP {

/* -------------------------------------------------------------------------- */

struct Values {
    static const std::map< int, std::string >         &error_code_to_message();
    static const std::map< std::string, std::string > &extension_to_mime();
};

/* -------------------------------------------------------------------------- */

struct Request : public Trait::Repr {
    std::string                          method;
    std::string                          url;
    std::string                          version;
    std::string                          host;
    std::map< std::string, std::string > header;

    virtual ~Request() {}

    std::ostream &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class DynamicParser {
public:
    enum e_step { REQUEST, HOST, HEADER, CONTENT, DONE };

private:
    std::string            _line;
    std::string            _sep;
    e_step                 _step;
    Ptr::shared< Request > _request;

public:
    DynamicParser();

    void                   operator<<( const std::string &s );
    e_step                 step() const;
    Ptr::shared< Request > request();

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

    Ptr::shared< Request >    _request;
    Ptr::shared< ServerConf > _conf;
    Response                  _response;

    const ServerConf::Route &_route;
    std::string              _contentType;

public:
    RequestHandler( Ptr::shared< Request >    request,
                    Ptr::shared< ServerConf > conf );
    void response();
    void toRedir();
    void toDirListing();
    void getMethod();
    void postMethod();
    void deleteMethod();

    std::string    errorMessage( int nb );
    HTTP::Response getResponse();
    std::string    getContentType( std::string path );
    void           setResponse( int nb, std::string content );

private:
    std::string _get_path() const {
        std::string route( _conf->routes.lower_bound( _request->url ) );
        std::string url( _request->url );
        if ( url.size() and *url.rbegin() == '/' ) {
            url.erase( url.size() - 1 );
        }
        return _route.root + url;
    }
};

/* -------------------------------------------------------------------------- */

}
