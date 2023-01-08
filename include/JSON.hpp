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
    std::string &      operator*();
    const std::string &operator*() const;
    std::ostream &     repr( std::ostream &os ) const;
};

/* -------------------------------------------------------------------------- */

class Number : public Value {
    double _n;

public:
    Number( double n );
    Value *       clone() const;
    double &      operator*();
    double        operator*() const;
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
    template < typename T, typename I > class Iterator;

public:
    typedef Iterator< Value, std::vector< Value * >::iterator > iterator;
    typedef Iterator< const Value, std::vector< Value * >::const_iterator >
        const_iterator;

private:
    std::vector< Value * > _v;

    template < typename T, typename I > class Iterator {
    public:
        typedef T              value_type;
        typedef T &            reference;
        typedef T *            pointer;
        typedef std::ptrdiff_t difference_type;

    private:
        I _it;

    public:
        Iterator();
        Iterator( const I &it );

        I base() const { return _it; }

        Iterator        operator+( difference_type n ) const;
        friend Iterator operator+( difference_type lhs, const Iterator &rhs );
        Iterator        operator-( difference_type n ) const;
        difference_type operator-( const Iterator &other ) const;

        Iterator &operator++();
        Iterator &operator--();
        Iterator  operator++( int );
        Iterator  operator--( int );

        Iterator operator+=( difference_type n );
        Iterator operator-=( difference_type n );

        bool operator==( const const_iterator &rhs ) const;
        bool operator!=( const const_iterator &rhs ) const;
        bool operator>( const const_iterator &rhs ) const;
        bool operator<( const const_iterator &rhs ) const;
        bool operator>=( const const_iterator &rhs ) const;
        bool operator<=( const const_iterator &rhs ) const;

        reference operator*();
        typename Iterator< const T,
                           std::vector< Value * >::const_iterator >::reference
        operator*() const;

        reference operator[]( difference_type i );
        typename Iterator< const T,
                           std::vector< Value * >::const_iterator >::reference
        operator[]( difference_type i ) const;

        pointer operator->();
        typename Iterator< const T,
                           std::vector< Value * >::const_iterator >::pointer
        operator->() const;

        operator const_iterator() const;
    };

public:
    Array();
    Array( const Array &other );
    ~Array();

    Value *       clone() const;
    void          add( const Value &v );
    std::ostream &repr( std::ostream &os ) const;

    iterator       begin();
    const_iterator begin() const;
    iterator       end();
    const_iterator end() const;
};

/* -------------------------------------------------------------------------- */

class Boolean : public Value {
    bool _b;

public:
    Boolean( bool b );
    Value *       clone() const;
    bool &        operator*();
    bool          operator*() const;
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
    template < typename T > T static from_string( const std::string &s );

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