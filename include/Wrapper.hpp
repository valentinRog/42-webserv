#pragma once

/* -------------------------------------------------------------------------- */

template < typename T > struct CloneTraitCRTP { virtual T *clone() const = 0; };

/* -------------------------------------------------------------------------- */

template < typename T > class PolymorphicWrapper {
    T *_p;

public:
    PolymorphicWrapper( const T &v );
    PolymorphicWrapper( const PolymorphicWrapper &other );
    ~PolymorphicWrapper();
    PolymorphicWrapper &operator=( const PolymorphicWrapper &other );

    T &      operator*();
    const T &operator*() const;

    T *      operator->();
    const T *operator->() const;

    template < typename U > U &      unwrap();
    template < typename U > const U &unwrap() const;
};

/* -------------------------------------------------------------------------- */

#include "Wrapper.tpp"
