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

Option< std::pair< std::string, std::string > >
HTTP::Header::parse_line( const std::string &line ) {
    std::istringstream iss( line );
    std::string        k;
    std::string        v;
    iss >> k;
    if ( iss.fail() ) {
        return Option< std::pair< std::string, std::string > >();
    }
    k = k.substr( 0, k.size() - 1 );
    v = iss.str().substr( k.size() + 2, iss.str().size() );
    return Option< std::pair< std::string, std::string > >(
        std::make_pair( k, Str::trim_right( v, "\r" ) ) );
}

void HTTP::Header::add_raw( const std::string &raw ) {
    clear();
    std::vector< std::string > v;
    Str::split( v, raw, "\n" );
    for ( std::vector< std::string >::const_iterator it( v.begin() );
          it != v.end();
          it++ ) {
        std::pair< std::string, std::string > p = parse_line( *it ).unwrap();
        insert( p );
    }
}

bool HTTP::Header::check_field( const std::string &k,
                                const std::string &v ) const {
    const_iterator it( find( k ) );
    return it == end() ? false : it->second == v;
}

/* -------------------------------- Response -------------------------------- */

const BiMap< HTTP::Response::e_header_key, std::string > &
HTTP::Response::key_to_string() {
    struct f {
        static BiMap< e_header_key, std::string > init() {
            BiMap< e_header_key, std::string > m;
            m.insert( std::make_pair( HOST, "Host" ) );
            m.insert( std::make_pair( CONTENT_TYPE, "Content-Type" ) );
            m.insert( std::make_pair( CONTENT_LENGTH, "Content-Length" ) );
            m.insert( std::make_pair( LOCATION, "Location" ) );
            return m;
        }
    };
    static const BiMap< e_header_key, std::string > m( f::init() );
    return m;
}

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

/* --------------------------------- Request -------------------------------- */

const BiMap< HTTP::Request::e_header_key, std::string > &
HTTP::Request::key_to_string() {
    struct f {
        static BiMap< e_header_key, std::string > init() {
            BiMap< e_header_key, std::string >             m;
            typedef std::pair< e_header_key, std::string > value_type;
            m.insert( value_type( CONTENT_LENGTH, "Content-Length" ) );
            m.insert( value_type( TRANSFER_ENCODING, "Transfert-encoding" ) );
            m.insert( value_type( COOKIE, "Cookie" ) );
            m.insert( value_type( CONNECTION, "Connection" ) );
            return m;
        }
    };
    static const BiMap< e_header_key, std::string > m( f::init() );
    return m;
}

const BiMap< HTTP::Request::e_method, std::string > &
HTTP::Request::method_to_string() {
    struct f {
        static BiMap< HTTP::Request::e_method, std::string > init() {
            BiMap< HTTP::Request::e_method, std::string > m;
            m.insert( std::make_pair( GET, "GET" ) );
            m.insert( std::make_pair( POST, "POST" ) );
            m.insert( std::make_pair( DELETE, "DELETE" ) );
            return m;
        }
    };
    static const BiMap< HTTP::Request::e_method, std::string > m( f::init() );
    return m;
}

HTTP::Request::Request() : _keep_alive( false ) {}

HTTP::Request::e_method HTTP::Request::method() const { return _method; }

const std::string &HTTP::Request::url() const { return _url; }

const std::string &HTTP::Request::version() const { return _version; }

const std::string &HTTP::Request::host() const { return _host; }

const HTTP::Header &HTTP::Request::header() const { return _header; }

bool HTTP::Request::keep_alive() const { return _keep_alive; }

const std::string &HTTP::Request::content() const { return _content; }

size_t HTTP::Request::count_header( e_header_key k ) const {
    return _header.count( key_to_string().at( k ) );
}

const std::string &HTTP::Request::at_header( e_header_key k ) const {
    return _header.at( key_to_string().at( k ) );
}

bool HTTP::Request::check_header_field( e_header_key       k,
                                        const std::string &v ) const {
    return _header.check_field( key_to_string().at( k ), v );
}

/* -------------------------------------------------------------------------- */
