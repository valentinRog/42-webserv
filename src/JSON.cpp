#include "JSON.hpp"

/* -------------------------------------------------------------------------- */

JSON::Value::~Value() {}

/* -------------------------------------------------------------------------- */

JSON::String::String( const std::string &s ) : _s( s ) {}
const std::string &JSON::String::get() const { return _s; }

std::ostream &JSON::String::repr( std::ostream &os ) const {
    return os << "\"" << _s << "\"";
}

/* -------------------------------------------------------------------------- */

JSON::Number::Number( double n ) : _n( n ) {}
double JSON::Number::get() const { return _n; }

std::ostream &JSON::Number::repr( std::ostream &os ) const { return os << _n; }

/* -------------------------------------------------------------------------- */

JSON::Object::~Object() {
    for ( std::map< std::string, JSON::Value * >::iterator it( _m.begin() );
          it != _m.end();
          it++ ) {
        delete it->second;
    }
}

const std::map< std::string, JSON::Value * > &JSON::Object::get() const {
    return _m;
}

void JSON::Object::add( const std::string &k, JSON::Value *v ) { _m[k] = v; }

std::ostream &JSON::Object::repr( std::ostream &os ) const {
    os << "{";
    for ( std::map< std::string, Value * >::const_iterator it( _m.begin() );
          it != _m.end();
          it++ ) {
        if ( it != _m.begin() ) { os << ", "; }
        os << it->first << ": " << *it->second;
    }
    return os << "}";
}

/* -------------------------------------------------------------------------- */

JSON::Array::~Array() {
    for ( std::vector< JSON::Value * >::iterator it( _v.begin() );
          it != _v.end();
          it++ ) {
        delete *it;
    }
}

const std::vector< JSON::Value * > &JSON::Array::get() const { return _v; }

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

bool JSON::Boolean::get() const { return _b; }

std::ostream &JSON::Boolean::repr( std::ostream &os ) const { return os << _b; }

/* -------------------------------------------------------------------------- */

std::ostream &JSON::Null::repr( std::ostream &os ) const {
    return os << "Null";
}

/* -------------------------------------------------------------------------- */

std::ostream &operator<<( std::ostream &os, const JSON::Value &v ) {
    try {
        return dynamic_cast< const JSON::String & >( v ).repr( os );
    } catch ( const std::exception & ) {}
    try {
        return dynamic_cast< const JSON::Number & >( v ).repr( os );
    } catch ( const std::exception & ) {}
    try {
        return dynamic_cast< const JSON::Object & >( v ).repr( os );
    } catch ( const std::exception & ) {}
    try {
        return dynamic_cast< const JSON::Array & >( v ).repr( os );
    } catch ( const std::exception & ) {}
    try {
        return dynamic_cast< const JSON::Boolean & >( v ).repr( os );
    } catch ( const std::exception & ) {}
    try {
        return dynamic_cast< const JSON::Null & >( v ).repr( os );
    } catch ( const std::exception & ) {}
    return os << "What the hell is this ?";
}

/* -------------------------------------------------------------------------- */
