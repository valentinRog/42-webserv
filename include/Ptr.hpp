#pragma once

#include "common.h"

namespace Ptr {

/* -------------------------------------------------------------------------- */

template < typename T > class unique {
    T *ptr;

public:
    unique() : ptr( 0 ) {}
    explicit unique( T *p ) : ptr( p ) {}
    ~unique() {  delete ptr; }

    T *release() {
        T *tmp = ptr;
        ptr    = 0;
        return tmp;
    }

    void reset( T *p = 0 ) {
        if ( ptr != p ) {
            delete ptr;
            ptr = p;
        }
    }

    T &operator*() const { return *ptr; }
    T *operator->() const { return ptr; }

    private:
        unique(const unique&);
        unique& operator=(const unique&);
};

/* -------------------------------------------------------------------------- */

template < typename T > class shared {
    T   *ptr;
    int *ref_count;

public:
    shared( T *p = 0 ) : ptr( p ) {
        if ( ptr ) {
            ref_count  = new int;
            *ref_count = 1;
        } else
            ref_count = 0;
    }

    shared( const shared< T > &p ) : ptr( p.ptr ), ref_count( p.ref_count ) {
        if ( ptr ) ( *ref_count )++;
    }

    ~shared() {
        if ( ptr && ( --*ref_count == 0 ) ) {
            delete ptr;
            delete ref_count;
        }
    }

    shared< T > &operator=( const shared< T > &p ) {
        if ( ptr == p.ptr ) return *this;
        if ( ptr && ( --*ref_count == 0 ) ) {
            delete ptr;
            delete ref_count;
        }
        ptr       = p.ptr;
        ref_count = p.ref_count;
        if ( ptr ) ( *ref_count )++;
        return *this;
    }
    T &operator*() const { return *ptr; }
    T *operator->() const { return ptr; }
};

/* -------------------------------------------------------------------------- */

}

#include "Ptr.tpp"