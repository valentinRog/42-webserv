#pragma once

#include "common.h"

namespace JSON {

/* -------------------------------------------------------------------------- */

struct Value {
    virtual ~Value();
    virtual std::ostream &repr( std::ostream &os ) const = 0;
};

/* -------------------------------------------------------------------------- */

class String : public Value {
    std::string _s;

public:
    String( const std::string &s );
    const std::string &get() const;
    std::ostream      &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Number : public Value {
    double _n;

public:
    Number( double n );
    double        get() const;
    std::ostream &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Object : public Value {
    std::map< std::string, Value * > _m;

public:
    ~Object();
    const std::map< std::string, Value * > &get() const;
    void          add( const std::string &k, Value *v );
    std::ostream &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Array : public Value {
    std::vector< Value * > _v;

public:
    ~Array();
    const std::vector< Value * > &get() const;
    void                          add( Value *v );
    std::ostream                 &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Boolean : public Value {
    bool _b;

public:
    Boolean( bool b );
    bool          get() const;
    std::ostream &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Null : public Value {
public:
    std::ostream &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */
}

std::ostream &operator<<( std::ostream &os, const JSON::Value &v );

/* -------------------------------------------------------------------------- */
