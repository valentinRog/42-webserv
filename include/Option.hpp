#pragma once

#include "common.h"

template < typename T > class Option {
    bool _is_some;
    union {
        T    _data;
        char _sentinel;
    };

public:
    Option();
    Option( const T &value );
    Option &operator=( const T &value );
    bool    is_some() const;
    bool    is_none() const;

    T &      unwrap();
    const T &unwrap() const;
};

#include "Option.tpp"
