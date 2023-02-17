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
    enum e_key {
        CONTENT_LENGTH,
        TRANSFER_ENCODING,
        COOKIE,
        CONNECTION,
        HOST,
        LOCATION,
        CONTENT_TYPE
    };
    static const BiMap< e_key, std::string > &key_to_string();

    bool        check_field( const std::string &k, const std::string &v ) const;
    std::string get( const std::string &k,
                     const std::string &def = std::string() ) const;

    static Option< Header > from_string( const std::string &s );
};

/* -------------------------------- Response -------------------------------- */

struct Response : public Trait::Stringify {
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

/* --------------------------- ContentAccumulator --------------------------- */

class ContentAccumulator {
    size_t                     _content_length;
    size_t                     _body_max_size;
    Ptr::Shared< std::string > _content;
    bool                       _done;
    bool                       _failed;

public:
    ContentAccumulator( size_t content_length,
                        size_t body_max_size = SIZE_MAX );

    void feed( const char *first, const char *last );

    bool                       done() const;
    bool                       failed() const;
    Ptr::Shared< std::string > content() const;
};

/* --------------------------------- Request -------------------------------- */

class Request {
private:
    std::string                _method;
    std::string                _url;
    std::string                _version;
    std::string                _host;
    Header                     _header;
    Ptr::Shared< std::string > _content;

public:
    Request();

    const std::string         &method() const;
    const std::string         &url() const;
    const std::string         &version() const;
    const std::string         &host() const;
    const Header              &header() const;
    Ptr::Shared< std::string > content() const;

    void set_content( Ptr::Shared< std::string > content ) {
        _content = content;
    }

    static Option< Request > from_string( const std::string &request_line,
                                          const std::string &raw_header );
};

/* -------------------------------------------------------------------------- */
}
