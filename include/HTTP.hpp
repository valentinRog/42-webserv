#pragma once

#include "JSON.hpp"
#include "Ptr.hpp"
#include "ServerConf.hpp"
#include "common.h"

namespace HTTP {

/* -------------------------------------------------------------------------- */

struct Values {
    static const std::map< int, std::string > &error_code_to_message();
};

/* -------------------------------------------------------------------------- */

struct Request {
    std::string                          method;
    std::string                          url;
    std::string                          version;
    std::string                          host;
    std::map< std::string, std::string > header;

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
};

/* -------------------------------------------------------------------------- */

struct Response : public Trait::Stringify {
    enum e_header_key { HOST, CONTENT_TYPE, CONTENT_LENGTH, LOCATION };

    std::string                           version;
    std::string                           code;
    std::string                           outcome;
    std::map< e_header_key, std::string > header;
    std::string                           content;

    std::string stringify() const;

private:
    static const std::map< e_header_key, std::string > &_header_key_name();
};

/* -------------------------------------------------------------------------- */

class RequestHandler {
    Ptr::shared< Request >    _request;
    Ptr::shared< ServerConf > _conf;
    Response                  _response;
    const ServerConf::Route * _route;
    std::string               _path;

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
    std::string    getContentType( const std::string &path );
    void           setResponse( int nb, std::string content );
};

/* -------------------------------------------------------------------------- */

}
