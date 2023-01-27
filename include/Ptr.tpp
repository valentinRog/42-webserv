#include "Ptr.hpp"

/* -------------------------------------------------------------------------- */

template < class T > inline Ptr::unique< T >::unique() : ptr( 0 ) {}

template < class T > Ptr::unique< T >::unique( T *p ) : ptr( p ) {}

template < class T > inline Ptr::unique< T >::~unique() { delete ptr; }

template < class T > inline T *Ptr::unique< T >::release() {
    T *tmp = ptr;
    ptr    = 0;
    return tmp;
}

template < class T > inline void Ptr::unique< T >::reset( T *p ) {
    if ( ptr != p ) {
        delete ptr;
        ptr = p;
    }
}
template < class T > inline T &Ptr::unique< T >::operator*() const {
    return *ptr;
}

template < class T > inline T *Ptr::unique< T >::operator->() const {
    return ptr;
}

/* -------------------------------------------------------------------------- */

template < class T > inline Ptr::shared< T >::shared( T *p ) : ptr( p ) {
    if ( ptr ) {
        ref_count  = new int;
        *ref_count = 1;
    } else
        ref_count = 0;
}

template < class T >
inline Ptr::shared< T >::shared( const Ptr::shared< T > &p )
    : ptr( p.ptr ),
      ref_count( p.ref_count ) {

    if ( ptr ) ( *ref_count )++;
}

template < class T > inline Ptr::shared< T >::~shared() {
    if ( ptr && ( --*ref_count == 0 ) ) {
        delete ptr;
        delete ref_count;
    }
}

template < class T >
inline Ptr::shared< T > &
Ptr::shared< T >::operator=( const Ptr::shared< T > &p ) {
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

template < class T > inline T &Ptr::shared< T >::operator*() const {
    return *ptr;
}

template < class T > inline T *Ptr::shared< T >::operator->() const {
    return ptr;
}

/* -------------------------------------------------------------------------- */
