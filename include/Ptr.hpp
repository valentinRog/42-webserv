#pragma once

#include "common.h"

namespace Ptr {

/* --------------------------------- Unique --------------------------------- */

template < typename T > class Unique {
    T *_p;

public:
    Unique();
    explicit Unique( T *p );
    ~Unique();

    T   *release();
    void reset( T *p = 0 );

    T       &operator*();
    const T &operator*() const;
    T       *operator->();
    const T *operator->() const;

private:
    Unique( const Unique & );
    Unique &operator=( const Unique & );
};

/* --------------------------------- Shared --------------------------------- */

template < typename T > class Shared {
    T   *_p;
    int *_n_ref;

public:
    Shared( T *p = 0 );
    Shared( const Shared< T > &p );
    Shared< T > &operator=( const Shared< T > &p );
    ~Shared();

    T       &operator*();
    const T &operator*() const;
    T       *operator->();
    const T *operator->() const;
};

/* -------------------------------------------------------------------------- */

}

#include "Ptr.tpp"