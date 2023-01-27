#pragma once

#include "common.h"

namespace Ptr {

/* -------------------------------------------------------------------------- */

template < typename T > class unique {
    T *_p;

public:
    unique();
    explicit unique( T *p );
    ~unique();

    T   *release();
    void reset( T *p = 0 );

    T       &operator*();
    const T &operator*() const;
    T       *operator->();
    const T *operator->() const;

private:
    unique( const unique & );
    unique &operator=( const unique & );
};

/* -------------------------------------------------------------------------- */

template < typename T > class shared {
    T   *_p;
    int *_n_ref;

public:
    shared( T *p = 0 );
    shared( const shared< T > &p );
    shared< T > &operator=( const shared< T > &p );
    ~shared();

    T       &operator*();
    const T &operator*() const;
    T       *operator->();
    const T *operator->() const;
};

/* -------------------------------------------------------------------------- */

}

#include "Ptr.tpp"