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

class Wrapper {
    Value *_v;

public:
    Wrapper();
    Wrapper( const Value &v );
    Wrapper( const Wrapper &other );
    ~Wrapper();
    Wrapper &operator=( const Wrapper &other );

    Value &                          unwrap();
    const Value &                    unwrap() const;
    template < typename T > T &      unwrap();
    template < typename T > const T &unwrap() const;
};

/* -------------------------------------------------------------------------- */

class String : public Value {
    std::string _s;

public:
    String( const std::string &s );
    Value *       clone() const;
    std::ostream &repr( std::ostream &os ) const;

    operator std::string() const;
};

/* -------------------------------------------------------------------------- */

class Number : public Value {
    double _n;

public:
    Number( double n );
    Value *       clone() const;
    std::ostream &repr( std::ostream &os ) const;

    operator double() const;
};

/* -------------------------------------------------------------------------- */

struct Object : public Value, public std::map< std::string, Wrapper > {
    Value *       clone() const;
    std::ostream &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

struct Array : public Value, public std::vector< Wrapper > {
    Value *       clone() const;
    std::ostream &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Boolean : public Value {
    bool _b;

public:
    Boolean( bool b );
    Value *       clone() const;
    std::ostream &repr( std::ostream &os ) const;

    operator bool() const;
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
    static Wrapper                   _parse( std::queue< std::string > &q );
    static String  _parse_string( std::queue< std::string > &q );
    static Number  _parse_number( std::queue< std::string > &q );
    static Object  _parse_object( std::queue< std::string > &q );
    static Array   _parse_array( std::queue< std::string > &q );
    static Boolean _parse_boolean( std::queue< std::string > &q );
    static Null    _parse_null( std::queue< std::string > &q );

public:
    static Wrapper from_string( const std::string &s );

    class ParsingError : public std::exception {
        virtual const char *what() const throw();
    };
};

/* -------------------------------------------------------------------------- */

}

/* ------------------------------------------------------------------------- */

std::ostream &operator<<( std::ostream &os, const JSON::Value &v );

/* -------------------------------------------------------------------------- */

#include "JSON.tpp"