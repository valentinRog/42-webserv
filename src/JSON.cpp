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

JSON::Parse::Parse( const std::string &s ) : _q( _lexer( s ) ) {}

JSON::Value *JSON::Parse::_json() {
    if ( !_q.size() ) { throw std::runtime_error( "" ); }
    if ( _q.front() == "{" ) {
        Object o;
        _q.pop_front();
        while ( _q.size() && _q.front() != "}" ) {
            std::string tok( _q.front() );
            if ( tok.size() < 3 || tok.front() != quote
                 || tok.back() != quote ) {
                throw std::runtime_error( "" );
            }
            _q.pop_front();
            std::string k = tok;
            if ( !_q.size() || _q.front() != ":" ) {
                throw std::runtime_error( "" );
            }
            _q.pop_front();
            if ( !_q.size() ) { throw std::runtime_error( "error" ); }
            Value *v = _json();
            o.add( k, *v );
        }
        _q.pop_front();
        return o.clone();
    }
    return String("yo").clone();
}

JSON::Object JSON::Parse::from_string( const std::string &s ) {
    return *dynamic_cast< Object * >( Parse( s )._json() );
}

/* -------------------------------------------------------------------------- */

std::ostream &operator<<( std::ostream &os, const JSON::Value &v ) {
    return v.repr( os );
}

/* -------------------------------------------------------------------------- */
