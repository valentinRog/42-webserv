#pragma once

#include "BiMap.hpp"
#include "JSON.hpp"
#include "Option.hpp"
#include "Ptr.hpp"
#include "Str.hpp"
#include "common.h"

namespace HTTP {

/* ---------------------------------- Mime ---------------------------------- */

struct Mime {
    static const std::map< std::string, std::string > &extension_to_type();
};

/* --------------------------------- Header --------------------------------- */

struct Header
    : public std::map< std::string, std::string, Str::CaseInsensitiveCmp > {
    static Option< std::pair< std::string, std::string > >
    parse_line( const std::string &line );

    void add_raw( const std::string &raw );
    bool check_field( const std::string &k, const std::string &v ) const;
};

/* ---------------------------------- Body ---------------------------------- */

class BodyParser {
    size_t      _body_max_size;
    std::string _content;

public:
    BodyParser( size_t body_max_size = SIZE_MAX )
        : _body_max_size( body_max_size ) {}

    void feed( const std::string &s );
};

/* -------------------------------- Response -------------------------------- */

struct Response : public Trait::Stringify {
    enum e_header_key { HOST, CONTENT_TYPE, CONTENT_LENGTH, LOCATION };
    static const BiMap< e_header_key, std::string > &key_to_string();

    enum e_error_code {
        E200,
        E301,
        E400,
        E403,
        E404,
        E405,
        E408,
        E413,
        E414,
        E500,
        E502,
        E505
    };
    static const BiMap< e_error_code, std::string > &code_to_string();
    static const std::string &code_to_message( e_error_code code );

    static const std::string &version();

    static Response make_error_response( e_error_code code );
    static Response make_error_response(
        e_error_code                                 code,
        const std::map< e_error_code, std::string > &error_pages );

    e_error_code code;
    Header       header;

    Response( e_error_code code );

    void        set_content( const std::string &s );
    std::string stringify() const;

private:
    std::string _content;
};

/* --------------------------------- Request -------------------------------- */

class Request {
public:
    enum e_header_key { CONTENT_LENGTH, TRANSFER_ENCODING, COOKIE, CONNECTION };
    static const BiMap< e_header_key, std::string > &key_to_string();

    enum e_method { GET, POST, DELETE };
    static const BiMap< e_method, std::string > &method_to_string();

private:
    e_method    _method;
    std::string _url;
    std::string _version;
    std::string _host;
    Header      _header;
    bool        _keep_alive;
    std::string _content;

public:
    Request();

    e_method           method() const;
    const std::string &url() const;
    const std::string &version() const;
    const std::string &host() const;
    const Header      &header() const;
    bool               keep_alive() const;
    const std::string &content() const;

    size_t             count_header( e_header_key k ) const;
    const std::string &at_header( e_header_key k ) const;
    bool check_header_field( e_header_key k, const std::string &v ) const;

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
        size_t                                 _content_overflow;
        Ptr::Shared< Request >                 _request;
        size_t                                 _content_length;
        std::string                            _raw_header;
        bool                                   _chunked;
        Option< HTTP::Response::e_error_code > _error;
        size_t                                 _max_body_size;

    public:
        DynamicParser( size_t max_body_size = SIZE_MAX );

        void                   add( const char *s, size_t n );
        bool                   done() const;
        bool                   failed() const;
        Ptr::Shared< Request > request() const;

        HTTP::Response::e_error_code error() const;

    private:
        void _parse_line();
        void _parse_request_line();
        void _parse_host_line();
        void _parse_header_line();
        void _parse_chunk_size_line();
        void _append_to_content( const char *s, size_t n );
    };

    /* -------------------------------------------------------------------------- */
};

/* -------------------------------------------------------------------------- */

}
