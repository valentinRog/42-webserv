#include "HTTP.hpp"

/* --------------------------------- Header --------------------------------- */

void HTTP::Header::add_raw( const std::string &raw ) {
    clear();
    std::vector< std::string > v;
    Str::split( v, raw, "\n" );
    for ( std::vector< std::string >::const_iterator it( v.begin() );
          it != v.end();
          it++ ) {
        std::istringstream iss( *it );
        std::string        k;
        std::string        v;
        iss >> k;
        k            = k.substr( 0, k.size() - 1 );
        v            = iss.str().substr( k.size() + 2, iss.str().size() );
        ( *this )[k] = v;
    }
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

const std::pair< std::string, std::string > &
HTTP::Response::code_to_string( HTTP::Response::e_error_code code ) {
    struct f {
        static std::map< e_error_code, std::pair< std::string, std::string > >
        init() {
            std::map< e_error_code, std::pair< std::string, std::string > > m;
            m[E200] = std::make_pair( "200", "OK" );
            m[E301] = std::make_pair( "301", "Move Permanently" );
            m[E400] = std::make_pair( "400", "Bad Request" );
            m[E403] = std::make_pair( "403", "Forbidden" );
            m[E404] = std::make_pair( "404", "Not Found" );
            m[E405] = std::make_pair( "405", "Method Not Allowed" );
            m[E408] = std::make_pair( "408", "Request Timeout" );
            m[E413] = std::make_pair( "413", "Request Entity Too Large" );
            m[E500] = std::make_pair( "500", "Internal Server Error" );
            m[E502] = std::make_pair( "502", "Bad Gateway" );
            m[E505] = std::make_pair( "505", "Version Not Supported" );
            return m;
        }
    };
    static const std::map< e_error_code, std::pair< std::string, std::string > >
        m( f::init() );
    return m.at( code );
}

const std::map< std::string, HTTP::Response::e_error_code > &
HTTP::Response::string_to_code() {
    typedef std::map< std::string, e_error_code > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[code_to_string( E200 ).first] = E200;
            m[code_to_string( E301 ).first] = E301;
            m[code_to_string( E400 ).first] = E400;
            m[code_to_string( E403 ).first] = E403;
            m[code_to_string( E404 ).first] = E404;
            m[code_to_string( E405 ).first] = E405;
            m[code_to_string( E408 ).first] = E408;
            m[code_to_string( E413 ).first] = E413;
            m[code_to_string( E500 ).first] = E500;
            m[code_to_string( E502 ).first] = E502;
            m[code_to_string( E505 ).first] = E505;
            return m;
        }
    };
    static const map_type m( f::init() );
    return m;
}

const std::string &HTTP::Response::version() {
    static const std::string s( "HTTP/1.1" );
    return s;
}

HTTP::Response
HTTP::Response::make_error_response( HTTP::Response::e_error_code code ) {
    Response r( code );
    r.set_content( "<h1>" + Response::code_to_string( code ).first + "</h1>"
                   + "<p>" + Response::code_to_string( code ).second + "</p>" );
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

std::string HTTP::Response::stringify() const {
    std::string s( version() + ' ' + code_to_string( code ).first + ' '
                   + code_to_string( code ).second + "\r\n" );
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
            BiMap< e_header_key, std::string > m;
            m.insert( std::make_pair( CONTENT_LENGTH, "Content-Length" ) );
            m.insert(
                std::make_pair( TRANSFER_ENCODING, "Transfert-encoding" ) );
            m.insert( std::make_pair( COOKIE, "Cookie" ) );
            m.insert( std::make_pair( CONNECTION, "Connection" ) );
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

/* ------------------------- Request::DynamicParser ------------------------- */

HTTP::Request::DynamicParser::DynamicParser()
    : _step( REQUEST ),
      _request( new Request() ),
      _chunked( false ) {}

void HTTP::Request::DynamicParser::add( const char *s, size_t n ) {
    const char *p( s );
    while ( !( _step & ( DONE | FAILED ) ) && p < s + n ) {
        if ( _step != CONTENT ) {
            for ( ; p < s + n && !( _step & ( CONTENT | FAILED ) ); p++ ) {
                if ( *p == '\r' || *p == '\n' ) {
                    _sep += *p;
                    if ( _sep == "\r\n" ) {
                        _parse_line();
                        _line.clear();
                        _sep.clear();
                    }
                } else {
                    _line += *p;
                }
            }
        }
        if ( _step == CONTENT ) { p += _append_to_content( p, n - ( p - s ) ); }
    }
}

HTTP::Request::DynamicParser::e_step
HTTP::Request::DynamicParser::step() const {
    return _step;
}

Ptr::Shared< HTTP::Request > HTTP::Request::DynamicParser::request() {
    return _request;
}

HTTP::Response::e_error_code HTTP::Request::DynamicParser::error() const {
    return _error.unwrap();
}

void HTTP::Request::DynamicParser::_parse_line() {
    switch ( _step ) {
    case REQUEST: _parse_request_line(); break;
    case HOST: _parse_host_line(); break;
    case HEADER: _parse_header_line(); break;
    case CHUNK_SIZE: _parse_chunk_size_line(); break;
    default: break;
    }
}

void HTTP::Request::DynamicParser::_parse_request_line() {
    std::istringstream iss( _line );
    std::string        s;
    iss >> s;
    _request->_method = Request::method_to_string().at( s );
    iss >> _request->_url;
    iss >> _request->_version;
    _step = HOST;
}

void HTTP::Request::DynamicParser::_parse_host_line() {
    std::istringstream iss( _line );
    iss.ignore( std::numeric_limits< std::streamsize >::max(), ' ' );
    iss >> _request->_host;
    _step = HEADER;
}

void HTTP::Request::DynamicParser::_parse_header_line() {
    std::istringstream iss( _line );
    if ( !iss.str().size() ) {
        _request->_header.add_raw( _raw_header );
        if ( _request->header().count( key_to_string().at( CONNECTION ) )
             && _request->header().at( key_to_string().at( CONNECTION ) )
                    == "keep-alive" ) {
            _request->_keep_alive = true;
        }
        if ( _request->_header.count( key_to_string().at( TRANSFER_ENCODING ) )
             && _request->_header.at( key_to_string().at( TRANSFER_ENCODING ) )
                    == "chunked" ) {
            _chunked = true;
            _step    = CHUNK_SIZE;
        } else if ( _request->_header.count(
                        key_to_string().at( CONTENT_LENGTH ) ) ) {
            std::istringstream iss(
                _request->_header.at( key_to_string().at( CONTENT_LENGTH ) ) );
            iss >> _content_length;
            _step = CONTENT;
        } else {
            _step = DONE;
        }
    } else {
        _raw_header += _line + "\r\n";
    }
}

void HTTP::Request::DynamicParser::_parse_chunk_size_line() {
    _chunked = true;
    std::istringstream iss( _line );
    iss >> std::hex >> _content_length;
    std::cout << _content_length << std::endl;
    _step = _content_length ? CONTENT : DONE;
}

size_t HTTP::Request::DynamicParser::_append_to_content( const char *s,
                                                         size_t      n ) {
    if ( _request->_content.size() + n > 10 ) {
        _step  = FAILED;
        _error = Response::E413;
        return 0;
    }
    n = std::min( n, _content_length - _request->_content.size() );
    _request->_content.append( s, n );
    if ( _chunked ) {
        _step = CHUNK_SIZE;
    } else if ( _request->_content.size() >= _content_length ) {
        _step = DONE;
    }
    return n;
}

/* -------------------------------------------------------------------------- */
