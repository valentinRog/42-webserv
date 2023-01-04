#pragma once

#include "common.h"

namespace JSON {

/* -------------------------------------------------------------------------- */

struct Value {
    virtual ~Value();
    virtual std::ostream &repr( std::ostream &os ) const = 0;
    virtual Value        *clone() const                  = 0;
};

/* -------------------------------------------------------------------------- */

class String : public Value {
    std::string _s;

public:
    String( const std::string &s );
    String( const String &other );
    Value             *clone() const;
    const std::string &get() const;
    std::ostream      &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Number : public Value {
    double _n;

public:
    Number( double n );
    Number( const Number &other );
    Value        *clone() const;
    double        get() const;
    std::ostream &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Object : public Value {
    std::map< std::string, Value * > _m;

public:
    Object();
    Object( const Object &other );
    ~Object();
    Value                                  *clone() const;
    const std::map< std::string, Value * > &get() const;
    void          add( const std::string &k, const Value &v );
    std::ostream &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Array : public Value {
    std::vector< Value * > _v;

public:
    Array();
    Array( const Array &other );
    ~Array();
    Value                        *clone() const;
    const std::vector< Value * > &get() const;
    void                          add( const Value &v );
    std::ostream                 &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Boolean : public Value {
    bool _b;

public:
    Boolean( bool b );
    Boolean( const Boolean &other );
    Value        *clone() const;
    bool          get() const;
    std::ostream &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Null : public Value {
public:
    Null();
    Null( const Null &other );
    Value        *clone() const;
    std::ostream &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */
}

std::ostream &operator<<( std::ostream &os, const JSON::Value &v );

/* -------------------------------------------------------------------------- */
