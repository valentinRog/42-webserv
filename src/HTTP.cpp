#include "HTTP.hpp"

/* ---------------------------------- Mime ---------------------------------- */

const std::map< std::string, std::string > &HTTP::Mime::extension_to_type() {
    struct f {
        static std::map< std::string, std::string > init() {
            std::map< std::string, std::string > m;
            m["html"] = "text/html";
            m["css"]  = "text/css";
            m["js"]   = "application/javascript";
            m["json"] = "application/json";
            m["png"]  = "image/png";
            m["pdf"]  = "application/pdf";
            return m;
        }
    };
    static const std::map< std::string, std::string > m( f::init() );
    return m;
}

/* --------------------------------- Header --------------------------------- */

const BiMap< HTTP::Header::e_key, std::string > &HTTP::Header::key_to_string() {
    struct f {
        static BiMap< e_key, std::string > init() {
            BiMap< e_key, std::string > m;
            m.insert( std::make_pair( HOST, "Host" ) );
            m.insert( std::make_pair( CONTENT_TYPE, "Content-Type" ) );
            m.insert( std::make_pair( CONTENT_LENGTH, "Content-Length" ) );
            m.insert( std::make_pair( LOCATION, "Location" ) );
            m.insert(
                std::make_pair( TRANSFER_ENCODING, "Transfer-Encoding" ) );
            m.insert( std::make_pair( COOKIE, "Cookie" ) );
            m.insert( std::make_pair( CONNECTION, "Connection" ) );
            return m;
        }
    };
    static const BiMap< e_key, std::string > m( f::init() );
    return m;
}

bool HTTP::Header::check_field( const std::string &k,
                                const std::string &v ) const {
    const_iterator it( find( k ) );
    return it == end() ? false : it->second == v;
}

std::string HTTP::Header::get( const std::string &k,
                               const std::string &def ) const {
    const_iterator it( find( k ) );
    return it == end() ? def : it->second;
}

Option< HTTP::Header > HTTP::Header::from_string( const std::string &s ) {
    Header                     h;
    std::vector< std::string > v;
    Str::split( v, s, "\n" );
    for ( std::vector< std::string >::const_iterator it( v.begin() );
          it != v.end();
          ++it ) {
        std::vector< std::string > kv;
        Str::split( kv, *it, ":" );
        for ( std::vector< std::string >::iterator it( kv.begin() );
              it != kv.end();
              ++it ) {
            *it = Str::trim( *it, " \r" );
        }
        h[kv[0]] = kv[1];
    }
    return Option< Header >( h );
}

/* -------------------------------- Response -------------------------------- */

const BiMap< HTTP::Response::e_error_code, std::string > &
HTTP::Response::code_to_string() {
    struct f {
        static BiMap< e_error_code, std::string > init() {
            BiMap< e_error_code, std::string > m;
            m.insert( std::make_pair( E200, "200" ) );
            m.insert( std::make_pair( E301, "301" ) );
            m.insert( std::make_pair( E400, "400" ) );
            m.insert( std::make_pair( E403, "403" ) );
            m.insert( std::make_pair( E404, "404" ) );
            m.insert( std::make_pair( E405, "405" ) );
            m.insert( std::make_pair( E408, "408" ) );
            m.insert( std::make_pair( E413, "413" ) );
            m.insert( std::make_pair( E414, "414" ) );
            m.insert( std::make_pair( E500, "500" ) );
            m.insert( std::make_pair( E502, "502" ) );
            m.insert( std::make_pair( E505, "505" ) );
            return m;
        }
    };
    static const BiMap< e_error_code, std::string > m( f::init() );
    return m;
}

const std::string &
HTTP::Response::code_to_message( HTTP::Response::e_error_code code ) {
    struct f {
        static std::map< e_error_code, std::string > init() {
            std::map< e_error_code, std::string > m;
            m[E200] = "OK";
            m[E301] = "Move Permanently";
            m[E400] = "Bad Request";
            m[E403] = "Forbidden";
            m[E404] = "Not Found";
            m[E405] = "Method Not Allowed";
            m[E408] = "Request Timeout";
            m[E413] = "Request Entity Too Large";
            m[E414] = "URI Too Long";
            m[E500] = "Internal Server Error";
            m[E502] = "Bad Gateway";
            m[E505] = "Version Not Supported";
            return m;
        }
    };
    static const std::map< e_error_code, std::string > m( f::init() );
    return m.at( code );
}

const std::string &HTTP::Response::version() {
    static const std::string s( "HTTP/1.1" );
    return s;
}

HTTP::Response
HTTP::Response::make_error_response( HTTP::Response::e_error_code code ) {
    Response r( code );
    r.set_content( "<h1>" + Response::code_to_string().at( code ) + "</h1>"
                   + "<p>" + Response::code_to_message( code ) + "</p>" );
    return r;
}

HTTP::Response HTTP::Response::make_error_response(
    e_error_code                                 code,
    const std::map< e_error_code, std::string > &error_pages ) {
    if ( !error_pages.count( code ) ) { return make_error_response( code ); }
    std::string        page( error_pages.at( code ) );
    std::ostringstream oss;
    std::ifstream      f( page.c_str() );
    oss << f.rdbuf();
    Response r( code );
    r.set_content( oss.str() );
    return r;
}

HTTP::Response::Response( HTTP::Response::e_error_code code ) : code( code ) {}

void HTTP::Response::set_content( const std::string &s ) {
    _content                 = s;
    header["Content-Length"] = Str::from( _content.size() );
}

std::string HTTP::Response::stringify() const {
    std::string s( version() + ' ' + code_to_string().at( code ) + ' '
                   + code_to_message( code ) + "\r\n" );
    for ( std::map< std::string, std::string >::const_iterator it
          = header.begin();
          it != header.end();
          it++ ) {
        s += it->first + ": " + it->second + "\r\n";
    }
    return s + "\r\n" + _content;
}

/* --------------------------- ContentAccumulator --------------------------- */

HTTP::ContentAccumulator::ContentAccumulator( size_t content_length,
                                              size_t body_max_size )
    : _content_length( content_length ),
      _body_max_size( body_max_size ),
      _content( new std::string() ),
      _done( false ),
      _failed( false ) {}

void HTTP::ContentAccumulator::feed( const char *first, const char *last ) {
    for ( ; first != last && _content->size() < _content_length; ++first ) {
        *_content += *first;
    }
    _done   = _content->size() == _content_length;
    _failed = _content->size() > _body_max_size;
}

bool HTTP::ContentAccumulator::done() const { return _done; }

bool HTTP::ContentAccumulator::failed() const { return _failed; }

Ptr::Shared< std::string > HTTP::ContentAccumulator::content() const {
    return _content;
}

/* --------------------------------- Request -------------------------------- */

HTTP::Request::Request() {}

const std::string &HTTP::Request::method() const { return _method; }

const std::string &HTTP::Request::url() const { return _url; }

const std::string &HTTP::Request::version() const { return _version; }

const std::string &HTTP::Request::host() const { return _host; }

const HTTP::Header &HTTP::Request::header() const { return _header; }

Ptr::Shared< std::string > HTTP::Request::content() const { return _content; }

Option< HTTP::Request >
HTTP::Request::from_string( const std::string &request_line,
                            const std::string &raw_header ) {
    Request r;
    r._method  = request_line.substr( 0, request_line.find( ' ' ) );
    r._url     = request_line.substr( request_line.find( ' ' ) + 1,
                                  request_line.rfind( ' ' )
                                      - request_line.find( ' ' ) - 1 );
    r._version = request_line.substr( request_line.rfind( ' ' ) + 1 );
    r._header = Header::from_string( Str::trim_right( raw_header, "\r\n\r\n" ) )
                    .unwrap();
    return r;
}

/* -------------------------------------------------------------------------- */
