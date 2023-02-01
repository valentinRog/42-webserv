#pragma once

#include "Ptr.hpp"
#include "common.h"

/* --------------------------- PolymorphicWrapper --------------------------- */

template < typename T > class PolymorphicWrapper {
    Ptr::Unique< T > _p;

public:
    PolymorphicWrapper( const T &v );
    PolymorphicWrapper( const PolymorphicWrapper &other );
    PolymorphicWrapper &operator=( const PolymorphicWrapper &other );

    T       &operator*();
    const T &operator*() const;

    T       *operator->();
    const T *operator->() const;

    template < typename U > U       &unwrap();
    template < typename U > const U &unwrap() const;
};

/* -------------------------------------------------------------------------- */

#include "PolymorphicWrapper.tpp"
