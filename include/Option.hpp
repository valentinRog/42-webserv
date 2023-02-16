#pragma once

#include "common.h"

template < typename T > class Option {
    T   * _data;

public:
    Option();
    Option( const T &value );
    Option(const Option &other);
    ~Option();

    Option &operator=( const Option &other );

    bool    is_some() const;
    bool    is_none() const;

    T &      unwrap();
    const T &unwrap() const;
};

#include "Option.tpp"
