#include "HTTP.hpp"

/* -------------------------------- Response -------------------------------- */

const std::pair< std::string, std::string > &
HTTP::Response::error_code_to_string( HTTP::Response::e_error_code code ) {
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
HTTP::Response::string_to_error_code() {
    typedef std::map< std::string, e_error_code > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[error_code_to_string( E200 ).first] = E200;
            m[error_code_to_string( E301 ).first] = E301;
            m[error_code_to_string( E400 ).first] = E400;
            m[error_code_to_string( E403 ).first] = E403;
            m[error_code_to_string( E404 ).first] = E404;
            m[error_code_to_string( E405 ).first] = E405;
            m[error_code_to_string( E408 ).first] = E408;
            m[error_code_to_string( E413 ).first] = E413;
            m[error_code_to_string( E500 ).first] = E500;
            m[error_code_to_string( E502 ).first] = E502;
            m[error_code_to_string( E505 ).first] = E505;
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

std::string HTTP::Response::stringify() const {
    std::string s( version() + ' ' + error_code_to_string( code ).first + ' '
                   + error_code_to_string( code ).second + "\r\n" );
    for ( std::map< e_header_key, std::string >::const_iterator it
          = header.begin();
          it != header.end();
          it++ ) {
        s += _header_key_name().at( it->first ) + ": " + it->second + "\r\n";
    }
    return s + "\r\n" + _content;
}

const std::map< HTTP::Response::e_header_key, std::string > &
HTTP::Response::_header_key_name() {
    struct f {
        static std::map< e_header_key, std::string > init() {
            std::map< e_header_key, std::string > m;
            m[HOST]           = "Host";
            m[CONTENT_TYPE]   = "Content-Type";
            m[CONTENT_LENGTH] = "Content-Length";
            m[LOCATION]       = "Location";
            return m;
        }
    };
    static const std::map< e_header_key, std::string > m( f::init() );
    return m;
}

/* --------------------------------- Request -------------------------------- */

const std::string &HTTP::Request::key_to_string( e_header_key k ) {
    typedef std::map< e_header_key, std::string > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[CONTENT_LENGTH]    = "Content-Length";
            m[TRANSFER_ENCODING] = "Transfer-Encoding";
            m[COOKIE]            = "Cookie";
            return m;
        }
    };
    static const map_type m( f::init() );
    return m.at( k );
}

const std::
    map< std::string, HTTP::Request::e_header_key, Str::CaseInsensitiveCmp > &
    HTTP::Request::string_to_key() {
    typedef std::map< std::string, e_header_key, Str::CaseInsensitiveCmp >
        map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[key_to_string( CONTENT_LENGTH )]    = CONTENT_LENGTH;
            m[key_to_string( TRANSFER_ENCODING )] = TRANSFER_ENCODING;
            m[key_to_string( COOKIE )]            = COOKIE;
            return m;
        }
    };
    static const map_type m( f::init() );
    return m;
}

const std::string &
HTTP::Request::method_to_string( HTTP::Request::e_method method ) {
    typedef std::map< e_method, std::string > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[GET]    = "GET";
            m[POST]   = "POST";
            m[DELETE] = "DELETE";
            return m;
        }
    };
    static const map_type m( f::init() );
    return m.at( method );
}

const std::map< std::string, HTTP::Request::e_method > &
HTTP::Request::string_to_method() {
    typedef std::map< std::string, e_method > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[method_to_string( GET )]    = GET;
            m[method_to_string( POST )]   = POST;
            m[method_to_string( DELETE )] = DELETE;
            return m;
        }
    };
    static const map_type m( f::init() );
    return m;
}

HTTP::Request::e_method HTTP::Request::method() const { return _method; }

const std::string &HTTP::Request::url() const { return _url; }

const std::string &HTTP::Request::version() const { return _version; }

const std::string &HTTP::Request::host() const { return _host; }

const std::map< HTTP::Request::e_header_key, std::string > &
HTTP::Request::defined_header() const {
    return _defined_header;
}

const std::map< std::string, std::string, Str::CaseInsensitiveCmp > &
HTTP::Request::header() const {
    return _header;
}

const std::string &HTTP::Request::content() const { return _content; }

/* ------------------------- Request::DynamicParser ------------------------- */

HTTP::Request::DynamicParser::DynamicParser()
    : _step( REQUEST ),
      _request( new Request() ),
      _chunked( false ) {}

void HTTP::Request::DynamicParser::add( const char *s, size_t n ) {
    const char *p( s );
    while ( !( _step & ( DONE | FAILED ) ) ) {
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
        if ( _step == CONTENT ) { _append_to_content( p, n - ( p - s ) ); }
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
    _request->_method = Request::string_to_method().at( s );
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
        if ( _request->_defined_header.count( TRANSFER_ENCODING )
             && _request->_defined_header.at( TRANSFER_ENCODING )
                    == "chunked" ) {
            _chunked = true;
            _step    = CHUNK_SIZE;
        } else if ( _request->_defined_header.count( CONTENT_LENGTH ) ) {
            std::istringstream iss(
                _request->_defined_header.at( CONTENT_LENGTH ) );
            iss >> _content_length;
            _step = CONTENT;
        } else {
            _step = DONE;
        }
    } else {
        std::string k;
        std::string v;
        iss >> k;
        k = k.substr( 0, k.size() - 1 );
        v = iss.str().substr( k.size() + 2, iss.str().size() );
        if ( Request::string_to_key().count( k ) ) {
            _request->_defined_header[Request::string_to_key().at( k )] = v;
        } else {
            _request->_header[k] = v;
        }
    }
}

void HTTP::Request::DynamicParser::_parse_chunk_size_line() {
    _chunked = true;
    std::istringstream iss( _line );
    iss >> std::hex >> _content_length;
    std::cout << _content_length << std::endl;
    _step = _content_length ? CONTENT : DONE;
}

void HTTP::Request::DynamicParser::_append_to_content( const char *s,
                                                       size_t      n ) {
    if ( _request->_content.size() + n > 10 ) {
        _step  = FAILED;
        _error = Response::E413;
        return;
    }
    n = std::min( n, _content_length - _request->_content.size() );
    _request->_content.append( s, n );
    if ( _chunked ) {
        _step = CHUNK_SIZE;
    } else if ( _request->_content.size() >= _content_length ) {
        _step = DONE;
    }
}

/* -------------------------------------------------------------------------- */
