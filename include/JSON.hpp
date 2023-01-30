#pragma once

#include "Trait.hpp"
#include "PolymorphicWrapper.hpp"
#include "common.h"

namespace JSON {

/* -------------------------------------------------------------------------- */

struct Value : public Trait::CloneCRTP< Value >,
               public Trait::Stringify,
               public Trait::Repr {
    virtual ~Value();
    virtual Value *     clone() const     = 0;
    virtual std::string stringify() const = 0;
    std::ostream &      repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

typedef PolymorphicWrapper< Value > Wrapper;

/* -------------------------------------------------------------------------- */

class String : public Value {
    std::string _s;

public:
    String( const std::string &s );
    Value *     clone() const;
    std::string stringify() const;

    operator std::string() const;
};

/* -------------------------------------------------------------------------- */

class Number : public Value {
    double _n;

public:
    Number( double n );
    Value *     clone() const;
    std::string stringify() const;

    operator double() const;
};

/* -------------------------------------------------------------------------- */

struct Object : public Value, public std::map< std::string, Wrapper > {
    Value *     clone() const;
    std::string stringify() const;
};

/* -------------------------------------------------------------------------- */

struct Array : public Value, public std::vector< Wrapper > {
    Value *     clone() const;
    std::string stringify() const;
};

/* -------------------------------------------------------------------------- */

class Boolean : public Value {
    bool _b;

public:
    Boolean( bool b );
    Value *     clone() const;
    std::string stringify() const;

    operator bool() const;
};

/* -------------------------------------------------------------------------- */

class Null : public Value {
public:
    Value *     clone() const;
    std::string stringify() const;
};

/* -------------------------------------------------------------------------- */

class Parse {
    static const char                     quote = '"';
    static const std::set< char > &whitespaces();
    static const std::set< char > &tokens();

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
    static Wrapper from_file( const std::string &filename );

    class ParsingError : public std::exception {
        virtual const char *what() const throw();
    };
};

/* -------------------------------------------------------------------------- */

}

/* ------------------------------------------------------------------------- */
