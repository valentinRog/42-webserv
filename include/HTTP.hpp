#pragma once

#include "JSON.hpp"
#include "Ptr.hpp"
#include "ServerConf.hpp"
#include "common.h"

namespace HTTP {

/* -------------------------------------------------------------------------- */

struct Request {
    enum e_header_key { CONTENT_LENGTH };
    static const std::string &key_to_string( e_header_key k );
    static const std::map< std::string, e_header_key, Str::CaseInsensitiveCmp >
        &string_to_key();

    enum e_method { GET, POST, DELETE };
    static const std::string &method_to_string( e_method );
    static const std::map< std::string, e_method > &string_to_method();

    e_method                              method;
    std::string                           url;
    std::string                           version;
    std::string                           host;
    std::map< e_header_key, std::string > defined_header;
    std::map< std::string, std::string, Str::CaseInsensitiveCmp > header;
    std::basic_string< uint8_t >                                  content;

    class DynamicParser {
    public:
        enum e_step {
            REQUEST = 1 << 0,
            HOST    = 1 << 1,
            HEADER  = 1 << 2,
            CONTENT = 1 << 3,
            DONE    = 1 << 4,
            FAILED  = 1 << 5
        };

    private:
        std::string            _line;
        std::string            _sep;
        e_step                 _step;
        Ptr::shared< Request > _request;
        size_t                 _content_length;

    public:
        DynamicParser();

        void                   add( const u_char *s, size_t n );
        e_step                 step() const;
        Ptr::shared< Request > request();

    private:
        void _parse_line();
        void _parse_request_line();
        void _parse_host_line();
        void _parse_header_line();
        void _append_to_content( const u_char *s, size_t n );
    };
};

/* -------------------------------------------------------------------------- */

struct Response : public Trait::Stringify {
    enum e_header_key { HOST, CONTENT_TYPE, CONTENT_LENGTH, LOCATION };

    enum e_error_code {
        E200,
        E301,
        E400,
        E403,
        E404,
        E405,
        E408,
        E500,
        E502,
        E505
    };
    static const std::pair< std::string, std::string > &
    error_code_to_string( e_error_code code );

    std::string                           version;
    e_error_code                          code;
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

    std::string    errorMessage( HTTP::Response::e_error_code code );
    HTTP::Response getResponse();
    std::string    getContentType( const std::string &path );
    void setResponse( Response::e_error_code code, std::string content );
};

/* -------------------------------------------------------------------------- */

}
