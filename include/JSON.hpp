#pragma once

#include "common.h"

namespace JSON {

/* -------------------------------------------------------------------------- */

struct Value {
    virtual ~Value();
    virtual std::ostream &repr( std::ostream &os ) const = 0;
    virtual Value *       clone() const                  = 0;
};

/* -------------------------------------------------------------------------- */

class String : public Value {
    std::string _s;

public:
    String( const std::string &s );
    Value *            clone() const;
    const std::string &get() const;
    std::ostream &     repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Number : public Value {
    double _n;

public:
    Number( double n );
    Value *       clone() const;
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
    Value *                                 clone() const;
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
    Value *                       clone() const;
    const std::vector< Value * > &get() const;
    void                          add( const Value &v );
    std::ostream &                repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Boolean : public Value {
    bool _b;

public:
    Boolean( bool b );
    Value *       clone() const;
    bool          get() const;
    std::ostream &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Null : public Value {
public:
    Value *       clone() const;
    std::ostream &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Parse {
    static const std::string whitespaces;
    static const std::string tokens;
    static const char        quote;

    static std::queue< std::string > _lexer( const std::string &s );
    static Value *                   _parse( std::queue< std::string > &q );
    static String  _parse_string( std::queue< std::string > &q );
    static Number  _parse_number( std::queue< std::string > &q );
    static Object  _parse_object( std::queue< std::string > &q );
    static Array   _parse_array( std::queue< std::string > &q );
    static Boolean _parse_boolean( std::queue< std::string > &q );
    static Null    _parse_null( std::queue< std::string > &q );

public:
    static Value *from_string( const std::string &s );

    class ParsingError : public std::exception {
        virtual const char *what() const throw();
    };
};

/* -------------------------------------------------------------------------- */

}

/* ------------------------------------------------------------------------- */

std::ostream &operator<<( std::ostream &os, const JSON::Value &v );

/* -------------------------------------------------------------------------- */
