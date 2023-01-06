#include "JSON.hpp"

/* -------------------------------------------------------------------------- */

JSON::Value::~Value() {}

/* -------------------------------------------------------------------------- */

JSON::String::String( const std::string &s ) : _s( s ) {}

JSON::Value *JSON::String::clone() const { return new JSON::String( *this ); }

const std::string &JSON::String::get() const { return _s; }

std::ostream &JSON::String::repr( std::ostream &os ) const {
    return os << "\"" << _s << "\"";
}

/* -------------------------------------------------------------------------- */

JSON::Number::Number( double n ) : _n( n ) {}

JSON::Value *JSON::Number::clone() const { return new JSON::Number( *this ); }

double JSON::Number::get() const { return _n; }

std::ostream &JSON::Number::repr( std::ostream &os ) const { return os << _n; }

/* -------------------------------------------------------------------------- */

JSON::Object::Object() {}

JSON::Object::Object( const JSON::Object &other ) {
    for ( std::map< std::string, Value * >::const_iterator it(
              other._m.begin() );
          it != other._m.end();
          it++ ) {
        add( it->first, *it->second );
    }
}

JSON::Object::~Object() {
    for ( std::map< std::string, JSON::Value * >::iterator it( _m.begin() );
          it != _m.end();
          it++ ) {
        delete it->second;
    }
}

JSON::Value *JSON::Object::clone() const { return new JSON::Object( *this ); }

const std::map< std::string, JSON::Value * > &JSON::Object::get() const {
    return _m;
}

void JSON::Object::add( const std::string &k, const JSON::Value &v ) {
    _m[k] = v.clone();
}

std::ostream &JSON::Object::repr( std::ostream &os ) const {
    os << "{";
    for ( std::map< std::string, Value * >::const_iterator it( _m.begin() );
          it != _m.end();
          it++ ) {
        if ( it != _m.begin() ) { os << ", "; }
        os << '"' << it->first << '"' << ": " << *it->second;
    }
    return os << "}";
}

/* -------------------------------------------------------------------------- */

JSON::Array::Array() {}

JSON::Array::Array( const JSON::Array &other ) {
    for ( std::vector< Value * >::const_iterator it( other._v.begin() );
          it != other._v.end();
          it++ ) {
        add( **it );
    }
}

JSON::Array::~Array() {
    for ( std::vector< JSON::Value * >::iterator it( _v.begin() );
          it != _v.end();
          it++ ) {
        delete *it;
    }
}

JSON::Value *JSON::Array::clone() const { return new JSON::Array( *this ); }

const std::vector< JSON::Value * > &JSON::Array::get() const { return _v; }

void JSON::Array::add( const JSON::Value &v ) { _v.push_back( v.clone() ); }

std::ostream &JSON::Array::repr( std::ostream &os ) const {
    os << "[";
    for ( std::vector< Value * >::const_iterator it( _v.begin() );
          it != _v.end();
          it++ ) {
        if ( it != _v.begin() ) { os << ", "; }
        os << **it;
    }
    return os << "]";
}

/* -------------------------------------------------------------------------- */

JSON::Boolean::Boolean( bool b ) : _b( b ) {}

JSON::Value *JSON::Boolean::clone() const { return new JSON::Boolean( *this ); }

bool JSON::Boolean::get() const { return _b; }

std::ostream &JSON::Boolean::repr( std::ostream &os ) const {
    return os << std::boolalpha << _b;
}

/* -------------------------------------------------------------------------- */

JSON::Value *JSON::Null::clone() const { return new JSON::Null( *this ); }

std::ostream &JSON::Null::repr( std::ostream &os ) const {
    return os << "null";
}

/* -------------------------------------------------------------------------- */

const std::string JSON::Parse::whitespaces = " \t\n\r";
const std::string JSON::Parse::tokens      = "{}[],:";
const char        JSON::Parse::quote       = '"';

std::deque< std::string > JSON::Parse::_lexer( const std::string &s ) {
    std::deque< std::string > q;
    std::string               acc;
    bool                      in_quote( false );
    for ( std::string::const_iterator it = s.begin(); it != s.end(); it++ ) {
        if ( *it == quote && !in_quote ) {
            if ( acc.size() ) { q.push_back( acc ); }
            acc      = *it;
            in_quote = true;
        } else if ( *it == quote && in_quote ) {
            q.push_back( acc + *it );
            acc.clear();
            in_quote = false;
        } else if ( in_quote ) {
            acc += *it;
        } else if ( tokens.find( *it ) != std::string::npos ) {
            if ( acc.size() ) { q.push_back( acc ); }
            q.push_back( std::string( 1, *it ) );
            acc.clear();
        } else if ( whitespaces.find( *it ) == std::string::npos ) {
            acc += *it;
        }
    }
    if ( acc.size() ) { q.push_back( acc ); }
    return q;
}

JSON::Value *JSON::Parse::_parse( std::deque< std::string > &q ) {
    if ( !q.size() ) { throw std::runtime_error( "" ); }
    if ( q.front().front() == quote ) { return _parse_string( q ).clone(); }
    if ( std::isdigit( q.front().front() ) ) {
        return _parse_number( q ).clone();
    }
    if ( q.front() == "{" ) { return _parse_object( q ).clone(); }
    if ( q.front() == "[" ) { return _parse_array( q ).clone(); }
    if ( q.front() == "true" || q.front() == "false" ) {
        return _parse_boolean( q ).clone();
    }
    return _parse_null( q ).clone();
}

JSON::String JSON::Parse::_parse_string( std::deque< std::string > &q ) {
    if ( !q.size() ) { throw std::runtime_error( "" ); }
    std::string s( q.front().substr( 1, q.front().size() - 2 ) );
    q.pop_front();
    return String( s );
}

JSON::Number JSON::Parse::_parse_number( std::deque< std::string > &q ) {
    std::string s( q.front() );
    if ( !q.size() ) { throw std::runtime_error( "" ); }
    q.pop_front();
    std::stringstream ss( s );
    double            n;
    ss >> n;
    return Number( n );
}

JSON::Object JSON::Parse::_parse_object( std::deque< std::string > &q ) {
    if ( q.size() < 5 || q.front() != "{" ) { throw std::runtime_error( "" ); }
    Object o;
    q.pop_front();
    while ( q.size() && q.front() != "}" ) {
        std::string tok( q.front() );
        if ( tok.size() < 3 || tok.front() != quote || tok.back() != quote ) {
            throw std::runtime_error( "yo" );
        }
        q.pop_front();
        std::string k = tok.substr( 1, tok.size() - 2 );
        if ( q.front() != ":" ) { throw std::runtime_error( "" ); }
        q.pop_front();
        Value *v = _parse( q );
        o.add( k, *v );
        delete v;
        if ( q.front() == "," ) {
            if ( q.size() < 4 ) { throw std::runtime_error( "" ); }
            q.pop_front();
        }
    }
    q.pop_front();
    return o;
}

JSON::Array JSON::Parse::_parse_array( std::deque< std::string > &q ) {
    if ( q.size() < 3 || q.front() != "[" ) { throw std::runtime_error( "" ); }
    Array a;
    q.pop_front();
    while ( q.front() != "]" ) {
        Value *v( _parse( q ) );
        a.add( *v );
        delete v;
        if ( q.front() == "," ) {
            if ( q.size() < 3 ) { throw std::runtime_error( "" ); }
            q.pop_front();
        }
    }
    q.pop_front();
    return a;
}

JSON::Boolean JSON::Parse::_parse_boolean( std::deque< std::string > &q ) {
    std::string s( q.front() );
    if ( !q.size() || ( s != "true" && s != "false" ) ) {
        throw std::runtime_error( "" );
    }
    q.pop_front();
    return s == "true" ? Boolean( true ) : Boolean( false );
}

JSON::Null JSON::Parse::_parse_null( std::deque< std::string > &q ) {
    if ( !q.size() ) { throw std::runtime_error( "" ); }
    q.pop_front();
    return Null();
}

JSON::Object JSON::Parse::from_string( const std::string &s ) {
    std::deque< std::string > q( _lexer( s ) );
    return *dynamic_cast< Object * >( _parse( q ) );
}

/* -------------------------------------------------------------------------- */

std::ostream &operator<<( std::ostream &os, const JSON::Value &v ) {
    return v.repr( os );
}

/* -------------------------------------------------------------------------- */
