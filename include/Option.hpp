#include "common.h"

template < typename T > class Option {
    bool _is_some;
    union {
        T    _data;
        char _sentinel;
    };

public:
    Option() : _is_some( false ) {}

    Option( const T &value ) : _is_some( true ) { _data = value; }

    Option &operator=( const T &value ) {
        _data    = value;
        _is_some = true;
        return *this;
    }

    bool is_some() const { return _is_some; }

    bool is_none() const { return !_is_some; }

    T unwrap() const {
        if ( !_is_some ) { throw std::runtime_error( "Option is None" ); }
        return _data;
    }
};
