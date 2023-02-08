#pragma once

#include "PolymorphicWrapper.hpp"
#include "Trait.hpp"
#include "common.h"

namespace JSON {

/* ---------------------------------- Value --------------------------------- */

struct Value : public Trait::CloneCRTP< Value >,
               public Trait::Stringify,
               public Trait::Repr {
    virtual ~Value();
    virtual Value      *clone() const     = 0;
    virtual std::string stringify() const = 0;
    std::ostream       &repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

typedef PolymorphicWrapper< Value > Wrapper;

/* --------------------------------- String --------------------------------- */

class String : public Value {
    std::string _s;

public:
    String( const std::string &s );
    Value      *clone() const;
    std::string stringify() const;

    operator std::string() const;
};

/* --------------------------------- Number --------------------------------- */

class Number : public Value {
    size_t _n;

public:
    Number( size_t n );
    Value      *clone() const;
    std::string stringify() const;

    operator size_t() const;
};

/* --------------------------------- Object --------------------------------- */

struct Object : public Value, public std::map< std::string, Wrapper > {
    Value      *clone() const;
    std::string stringify() const;
};

/* ---------------------------------- Array --------------------------------- */

struct Array : public Value, public std::vector< Wrapper > {
    Value      *clone() const;
    std::string stringify() const;
};

/* --------------------------------- Boolean -------------------------------- */

class Boolean : public Value {
    bool _b;

public:
    Boolean( bool b );
    Value      *clone() const;
    std::string stringify() const;

    operator bool() const;
};

/* ---------------------------------- Null ---------------------------------- */

class Null : public Value {
public:
    Value      *clone() const;
    std::string stringify() const;
};

/* ---------------------------------- Parse --------------------------------- */

class Parse {
    static const char              quote = '"';
    static const std::set< char > &whitespaces();
    static const std::set< char > &tokens();

    Parse();

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

    /* --------------------------- Parse::ParsingError -------------------------- */

    class ParsingError : public std::exception {
        virtual const char *what() const throw();
    };

    /* -------------------------------------------------------------------------- */
};

/* -------------------------------------------------------------------------- */

}

/* ------------------------------------------------------------------------- */
