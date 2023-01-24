#include "JSON.hpp"

/* -------------------------------------------------------------------------- */

JSON::Value::~Value() {}

std::ostream &JSON::Value::repr(std::ostream &os) const {
    return os << stringify();
}

/* -------------------------------------------------------------------------- */

JSON::String::String( const std::string &s ) : _s( s ) {}

JSON::Value *JSON::String::clone() const { return new JSON::String( *this ); }

std::string JSON::String::stringify() const { return '"' + _s + '"'; }

JSON::String::operator std::string() const { return _s; }

/* -------------------------------------------------------------------------- */

JSON::Number::Number( double n ) : _n( n ) {}

JSON::Value *JSON::Number::clone() const { return new JSON::Number( *this ); }

std::string JSON::Number::stringify() const {
    std::ostringstream oss;
    oss << _n;
    return oss.str();
}

JSON::Number::operator double() const { return _n; }

/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */

JSON::Boolean::Boolean( bool b ) : _b( b ) {}

JSON::Value *JSON::Boolean::clone() const { return new JSON::Boolean( *this ); }

std::string JSON::Boolean::stringify() const {
    std::ostringstream oss;
    oss << std::boolalpha << _b;
    return oss.str();
}

JSON::Boolean::operator bool() const { return _b; }

/* -------------------------------------------------------------------------- */

JSON::Value *JSON::Null::clone() const { return new JSON::Null( *this ); }

std::string JSON::Null::stringify() const { return "null"; }

/* -------------------------------------------------------------------------- */

const std::string JSON::Parse::whitespaces = " \t\n\r";
const std::string JSON::Parse::tokens      = "{}[],:";
const char        JSON::Parse::quote       = '"';

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
        } else if ( tokens.find( *it ) != std::string::npos ) {
            if ( acc.size() ) { q.push( acc ); }
            q.push( std::string( 1, *it ) );
            acc.clear();
        } else if ( whitespaces.find( *it ) == std::string::npos ) {
            acc += *it;
        }
    }
    if ( acc.size() ) { q.push( acc ); }
    return q;
}

JSON::Wrapper JSON::Parse::_parse( std::queue< std::string > &q ) {
    if ( !q.size() ) { throw ParsingError(); }
    if ( q.front()[0] == quote ) { return _parse_string( q ); }
    if ( std::isdigit( q.front()[0] ) ) { return _parse_number( q ); }
    if ( q.front() == "{" ) { return _parse_object( q ); }
    if ( q.front() == "[" ) { return _parse_array( q ); }
    if ( q.front() == "true" || q.front() == "false" ) {
        return _parse_boolean( q );
    }
    return _parse_null( q );
}

JSON::String JSON::Parse::_parse_string( std::queue< std::string > &q ) {
    if ( !q.size() ) { throw ParsingError(); }
    std::string s( q.front() );
    if ( s.size() < 2 || s[0] != quote || s[0] != s[s.size() - 1] ) {
        throw ParsingError();
    }
    s = q.front().substr( 1, q.front().size() - 2 );
    q.pop();
    return String( s );
}

JSON::Number JSON::Parse::_parse_number( std::queue< std::string > &q ) {
    std::string s( q.front() );
    if ( !q.size() ) { throw ParsingError(); }
    q.pop();
    std::stringstream ss( s );
    double            n;
    ss >> n;
    return Number( n );
}

JSON::Object JSON::Parse::_parse_object( std::queue< std::string > &q ) {
    if ( q.size() < 2 || q.front() != "{" ) { throw ParsingError(); }
    Object o;
    q.pop();
    while ( q.size() && q.front() != "}" ) {
        std::string tok( q.front() );
        if ( tok.size() < 3 || tok[0] != quote
             || tok[tok.size() - 1] != quote ) {
            throw ParsingError();
        }
        q.pop();
        std::string k = tok.substr( 1, tok.size() - 2 );
        if ( q.front() != ":" ) { throw ParsingError(); }
        q.pop();
        o.insert( Object::value_type( k, _parse( q ) ) );
        if ( q.front() == "," ) {
            if ( q.size() < 4 ) { throw ParsingError(); }
            q.pop();
        }
    }
    q.pop();
    return o;
}

JSON::Array JSON::Parse::_parse_array( std::queue< std::string > &q ) {
    if ( q.size() < 3 || q.front() != "[" ) { throw ParsingError(); }
    Array a;
    q.pop();
    while ( q.front() != "]" ) {
        a.push_back( _parse( q ) );
        if ( q.front() == "," ) {
            if ( q.size() < 3 ) { throw ParsingError(); }
            q.pop();
        } else if ( q.front() != "]" ) {
            throw ParsingError();
        }
    }
    q.pop();
    return a;
}

JSON::Boolean JSON::Parse::_parse_boolean( std::queue< std::string > &q ) {
    std::string s( q.front() );
    if ( !q.size() || ( s != "true" && s != "false" ) ) {
        throw ParsingError();
    }
    q.pop();
    return s == "true" ? Boolean( true ) : Boolean( false );
}

JSON::Null JSON::Parse::_parse_null( std::queue< std::string > &q ) {
    if ( !q.size() || q.front() != "null" ) { throw ParsingError(); }
    q.pop();
    return Null();
}

JSON::Wrapper JSON::Parse::from_string( const std::string &s ) {
    std::queue< std::string > q( _lexer( s ) );
    return _parse( q );
}

JSON::Wrapper JSON::Parse::from_file( const std::string &filename ) {
    std::stringstream ss;
    ss << std::ifstream( filename.c_str() ).rdbuf();
    std::queue< std::string > q( _lexer( ss.str() ) );
    return _parse( q );
}

const char *JSON::Parse::ParsingError::what() const throw() {
    return "Error while parsing JSON";
}

/* -------------------------------------------------------------------------- */
