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

JSON::Number::Number( const Number &other ) : _n( other._n ) {}

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
        os << it->first << ": " << *it->second;
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

JSON::Boolean::Boolean( const JSON::Boolean &other ) : _b( other._b ) {}

JSON::Value *JSON::Boolean::clone() const { return new JSON::Boolean( *this ); }

bool JSON::Boolean::get() const { return _b; }

std::ostream &JSON::Boolean::repr( std::ostream &os ) const {
    return os << std::boolalpha << _b;
}

/* -------------------------------------------------------------------------- */

JSON::Null::Null() {}

JSON::Null::Null( const Null & ) {}

JSON::Value *JSON::Null::clone() const { return new JSON::Null( *this ); }

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
