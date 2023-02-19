#include "JSON.hpp"

/* ---------------------------------- Value --------------------------------- */

JSON::Value::~Value() {}

std::ostream &JSON::Value::repr( std::ostream &os ) const {
    return os << stringify();
}

/* --------------------------------- String --------------------------------- */

JSON::String::String( const std::string &s ) : _s( s ) {}

JSON::Value *JSON::String::clone() const { return new JSON::String( *this ); }

std::string JSON::String::stringify() const { return '"' + _s + '"'; }

JSON::String::operator std::string() const { return _s; }

/* --------------------------------- Number --------------------------------- */

JSON::Number::Number( size_t n ) : _n( n ) {}

JSON::Value *JSON::Number::clone() const { return new JSON::Number( *this ); }

std::string JSON::Number::stringify() const {
    std::ostringstream oss;
    oss << _n;
    return oss.str();
}

JSON::Number::operator size_t() const { return _n; }

/* --------------------------------- Object --------------------------------- */

JSON::Value *JSON::Object::clone() const { return new JSON::Object( *this ); }

std::string JSON::Object::stringify() const {
    std::string s;
    s += '{';
    for ( const_iterator it( begin() ); it != end(); it++ ) {
        if ( it != begin() ) { s += ", "; }
        s += '"' + it->first + "\": " + it->second->stringify();
    }
    return s + '}';
}

/* ---------------------------------- Array --------------------------------- */

JSON::Value *JSON::Array::clone() const { return new JSON::Array( *this ); }

std::string JSON::Array::stringify() const {
    std::string s;
    s += '[';
    for ( const_iterator it( begin() ); it != end(); it++ ) {
        if ( it != begin() ) { s += ", "; }
        s += ( *it )->stringify();
    }
    return s + ']';
}

/* --------------------------------- Boolean -------------------------------- */

JSON::Boolean::Boolean( bool b ) : _b( b ) {}

JSON::Value *JSON::Boolean::clone() const { return new JSON::Boolean( *this ); }

std::string JSON::Boolean::stringify() const {
    std::ostringstream oss;
    oss << std::boolalpha << _b;
    return oss.str();
}

JSON::Boolean::operator bool() const { return _b; }

/* ---------------------------------- Null ---------------------------------- */

JSON::Value *JSON::Null::clone() const { return new JSON::Null( *this ); }

std::string JSON::Null::stringify() const { return "null"; }

/* ---------------------------------- Parse --------------------------------- */

const std::set< char > &JSON::Parse::whitespaces() {
    struct f {
        static std::set< char > init() {
            std::string s( " \t\n\r" );
            return std::set< char >( s.begin(), s.end() );
        }
    };
    static const std::set< char > s( f::init() );
    return s;
}

const std::set< char > &JSON::Parse::tokens() {
    struct f {
        static std::set< char > init() {
            std::string s( "{}[],:" );
            return std::set< char >( s.begin(), s.end() );
        }
    };
    static const std::set< char > s( f::init() );
    return s;
}

std::queue< std::string > JSON::Parse::_lexer( const std::string &s ) {
    std::queue< std::string > q;
    std::string               acc;
    bool                      in_quote( false );
    for ( std::string::const_iterator it = s.begin(); it != s.end(); it++ ) {
        if ( *it == quote && !in_quote ) {
            if ( acc.size() ) { q.push( acc ); }
            acc      = *it;
            in_quote = true;
        } else if ( *it == quote && in_quote ) {
            q.push( acc + *it );
            acc.clear();
            in_quote = false;
        } else if ( in_quote ) {
            acc += *it;
        } else if ( tokens().count( *it ) ) {
            if ( acc.size() ) { q.push( acc ); }
            q.push( std::string( 1, *it ) );
            acc.clear();
        } else if ( !whitespaces().count( *it ) ) {
            acc += *it;
        }
    }
    if ( acc.size() ) { q.push( acc ); }
    return q;
}

Option< JSON::Wrapper > JSON::Parse::_parse( std::queue< std::string > &q ) {
    const Option< JSON::Wrapper > fail;
    if ( !q.size() ) { return fail; }
    if ( q.front()[0] == quote ) {
        Option< String > o = _parse_string( q );
        return o.is_some() ? Option< JSON::Wrapper >( o.unwrap() ) : fail;
    }
    if ( std::isdigit( q.front()[0] ) ) {
        Option< Number > o = _parse_number( q );
        return o.is_some() ? Option< JSON::Wrapper >( o.unwrap() ) : fail;
    }
    if ( q.front() == "{" ) {
        Option< Object > o = _parse_object( q );
        return o.is_some() ? Option< JSON::Wrapper >( o.unwrap() ) : fail;
    }
    if ( q.front() == "[" ) {
        Option< Array > o = _parse_array( q );
        return o.is_some() ? Option< JSON::Wrapper >( o.unwrap() ) : fail;
    }
    if ( q.front() == "true" || q.front() == "false" ) {
        Option< Boolean > o = _parse_boolean( q );
        return o.is_some() ? Option< JSON::Wrapper >( o.unwrap() ) : fail;
    }
    Option< Null > o = _parse_null( q );
    return o.is_some() ? Option< JSON::Wrapper >( o.unwrap() ) : fail;
}

Option< JSON::String >
JSON::Parse::_parse_string( std::queue< std::string > &q ) {
    if ( !q.size() ) { return Option< JSON::String >(); }
    std::string s( q.front() );
    if ( s.size() < 2 || s[0] != quote || s[0] != s[s.size() - 1] ) {
        return Option< JSON::String >();
    }
    s = q.front().substr( 1, q.front().size() - 2 );
    q.pop();
    return String( s );
}

Option< JSON::Number >
JSON::Parse::_parse_number( std::queue< std::string > &q ) {
    std::string s( q.front() );
    if ( !q.size() ) { return Option< JSON::Number >(); }
    q.pop();
    size_t n;
    std::istringstream( s ) >> n;
    return Number( n );
}

Option< JSON::Object >
JSON::Parse::_parse_object( std::queue< std::string > &q ) {
    Option< Object > fail;
    if ( q.size() < 2 || q.front() != "{" ) { return fail; }
    Object o;
    q.pop();
    while ( q.size() && q.front() != "}" ) {
        std::string tok( q.front() );
        if ( tok.size() < 3 || tok[0] != quote
             || tok[tok.size() - 1] != quote ) {
            return fail;
        }
        q.pop();
        std::string k = tok.substr( 1, tok.size() - 2 );
        if ( q.front() != ":" ) { return fail; }
        q.pop();
        Option< JSON::Wrapper > o2 = _parse( q );
        if ( o2.is_none() ) { return fail; }
        o.insert( Object::value_type( k, o2.unwrap() ) );
        if ( q.front() == "," ) {
            if ( q.size() < 4 ) { return fail; }
            q.pop();
        }
    }
    q.pop();
    return o;
}

Option< JSON::Array >
JSON::Parse::_parse_array( std::queue< std::string > &q ) {
    Option< Array > fail;
    if ( q.size() < 3 || q.front() != "[" ) { return fail; }
    Array a;
    q.pop();
    while ( q.front() != "]" ) {
        Option< JSON::Wrapper > o = _parse( q );
        if ( o.is_none() ) { return fail; }
        a.push_back( o.unwrap() );
        if ( q.front() == "," ) {
            if ( q.size() < 3 ) { return fail; }
            q.pop();
        } else if ( q.front() != "]" ) {
            return fail;
        }
    }
    q.pop();
    return a;
}

Option< JSON::Boolean >
JSON::Parse::_parse_boolean( std::queue< std::string > &q ) {
    std::string s( q.front() );
    if ( !q.size() || ( s != "true" && s != "false" ) ) {
        return Option< JSON::Boolean >();
    }
    q.pop();
    return s == "true" ? Boolean( true ) : Boolean( false );
}

Option< JSON::Null > JSON::Parse::_parse_null( std::queue< std::string > &q ) {
    if ( !q.size() || q.front() != "null" ) { return Option< JSON::Null >(); }
    q.pop();
    return Null();
}

Option< JSON::Wrapper > JSON::Parse::from_string( const std::string &s ) {
    std::queue< std::string > q( _lexer( s ) );
    return _parse( q );
}

Option< JSON::Wrapper > JSON::Parse::from_file( const std::string &filename ) {
    std::ostringstream oss;
    oss << std::ifstream( filename.c_str() ).rdbuf();
    std::queue< std::string > q( _lexer( oss.str() ) );
    return _parse( q );
}

/* -------------------------------------------------------------------------- */
