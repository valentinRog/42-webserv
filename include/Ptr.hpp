#pragma once

#include "common.h"

namespace Ptr {

/* -------------------------------------------------------------------------- */

template < typename T > class unique {
    T *ptr;

public:
    unique();
    explicit unique( T *p );
    ~unique();

    T *release();

    void reset( T *p = 0 );

    T &operator*() const;
    T *operator->() const;

private:
    unique( const unique & );
    unique &operator=( const unique & );
};

/* -------------------------------------------------------------------------- */

template < typename T > class shared {
    T   *ptr;
    int *ref_count;

public:
    shared( T *p = 0 );

    shared( const shared< T > &p );

    ~shared();

    shared< T > &operator=( const shared< T > &p );
    T           &operator*() const;
    T           *operator->() const;
};

/* -------------------------------------------------------------------------- */

}

#include "Ptr.tpp"