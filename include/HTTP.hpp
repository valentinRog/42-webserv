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

class ContentAccumulator {
    size_t      _body_max_size;
    std::string _content;
    size_t      _content_length;
    bool        _done;

public:
    ContentAccumulator( size_t content_length, size_t body_max_size = SIZE_MAX )
        : _body_max_size( body_max_size ),
          _content_length( content_length ),
          _done( false ) {}

    void feed( const std::string &s ) {
        _content += s;
        if ( _content.size() >= _content_length ) {
            _content = _content.substr( 0, _content_length );
            _done    = true;
        }
    }

    bool               done() const { return _done; }
    const std::string &content() const { return _content; }
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
    e_method                   _method;
    std::string                _url;
    std::string                _version;
    std::string                _host;
    Header                     _header;
    bool                       _keep_alive;
    std::string                _content;
    Ptr::Shared< std::string > _content_ptr;

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

    static Option< Request > from_string( const std::string &request_line,
                                          const std::string &raw_header,
                                          const std::string &content ) {
        Request r;
        r._content = content;
        r._method  = method_to_string().at(
            request_line.substr( 0, request_line.find( ' ' ) ) );
        r._url     = request_line.substr( request_line.find( ' ' ) + 1,
                                      request_line.rfind( ' ' )
                                          - request_line.find( ' ' ) - 1 );
        r._version = request_line.substr( request_line.rfind( ' ' ) + 1 );
        r._header.add_raw( Str::trim_right( raw_header, "\r\n\r\n" ) );
        return r;
    }
};

/* -------------------------------------------------------------------------- */

}
