#pragma once

#include "JSON.hpp"
#include "Option.hpp"
#include "Ptr.hpp"
#include "Str.hpp"
#include "common.h"

namespace HTTP {

/* -------------------------------- Response -------------------------------- */

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
        E413,
        E500,
        E502,
        E505
    };
    static const std::pair< std::string, std::string > &
    error_code_to_string( e_error_code code );
    static const std::map< std::string, e_error_code > &string_to_error_code();

    static const std::string &version();

    Response( e_error_code code ) : code( code ) {}

    e_error_code                          code;
    std::map< e_header_key, std::string > header;

private:
    std::string _content;

public:
    void set_content( const std::string &s ) {
        _content               = s;
        header[CONTENT_LENGTH] = Str::from( _content.size() );
    }

    std::string stringify() const;

private:
    static const std::map< e_header_key, std::string > &_header_key_name();
};

/* --------------------------------- Request -------------------------------- */

class Request {
public:
    enum e_header_key { CONTENT_LENGTH, TRANSFER_ENCODING, COOKIE };
    static const std::string &key_to_string( e_header_key k );
    static const std::map< std::string, e_header_key, Str::CaseInsensitiveCmp >
        &string_to_key();

    enum e_method { GET, POST, DELETE };
    static const std::string &method_to_string( e_method );
    static const std::map< std::string, e_method > &string_to_method();

    e_method                              _method;
    std::string                           _url;
    std::string                           _version;
    std::string                           _host;
    std::map< e_header_key, std::string > _defined_header;
    std::map< std::string, std::string, Str::CaseInsensitiveCmp > _header;
    std::string                                                   _content;

public:
    e_method                                     method() const;
    const std::string                           &url() const;
    const std::string                           &version() const;
    const std::string                           &host() const;
    const std::map< e_header_key, std::string > &defined_header() const;
    const std::map< std::string, std::string, Str::CaseInsensitiveCmp >                    &
    header() const;
    const std::string &content() const;

    /* ------------------------- Request::DynamicParser ------------------------- */

    class DynamicParser {
    public:
        enum e_step {
            REQUEST    = 1 << 0,
            HOST       = 1 << 1,
            HEADER     = 1 << 2,
            CONTENT    = 1 << 3,
            DONE       = 1 << 4,
            FAILED     = 1 << 5,
            CHUNK_SIZE = 1 << 6
        };

    private:
        std::string                            _line;
        std::string                            _sep;
        e_step                                 _step;
        Ptr::Shared< Request >                 _request;
        size_t                                 _content_length;
        bool                                   _chunked;
        Option< HTTP::Response::e_error_code > _error;

    public:
        DynamicParser();

        void                   add( const char *s, size_t n );
        e_step                 step() const;
        Ptr::Shared< Request > request();

        HTTP::Response::e_error_code error() const;

    private:
        void   _parse_line();
        void   _parse_request_line();
        void   _parse_host_line();
        void   _parse_header_line();
        void   _parse_chunk_size_line();
        size_t _append_to_content( const char *s, size_t n );
    };

    /* -------------------------------------------------------------------------- */
};

/* -------------------------------------------------------------------------- */

}
