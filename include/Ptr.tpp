#include "Ptr.hpp"

/* --------------------------------- Unique --------------------------------- */

template < class T > Ptr::Unique< T >::Unique() : _p( 0 ) {}

template < class T > Ptr::Unique< T >::Unique( T *p ) : _p( p ) {}

template < class T > Ptr::Unique< T >::~Unique() { delete _p; }

template < class T > T *Ptr::Unique< T >::release() {
    T *tmp( _p );
    _p = 0;
    return tmp;
}

template < class T > void Ptr::Unique< T >::reset( T *p ) {
    if ( _p != p ) {
        delete _p;
        _p = p;
    }
}
template < class T > T &Ptr::Unique< T >::operator*() { return *_p; }

template < class T > const T &Ptr::Unique< T >::operator*() const {
    return *_p;
}

template < class T > T *Ptr::Unique< T >::operator->() { return _p; }

template < class T > const T *Ptr::Unique< T >::operator->() const {
    return _p;
}

/* --------------------------------- Shared --------------------------------- */

template < class T > Ptr::Shared< T >::Shared( T *p ) : _p( p ) {
    if ( _p ) {
        _n_ref = new int( 1 );
    } else {
        _n_ref = 0;
    }
}

template < class T >
Ptr::Shared< T >::Shared( const Ptr::Shared< T > &p )
    : _p( p._p ),
      _n_ref( p._n_ref ) {
    if ( _p ) { ( *_n_ref )++; }
}

template < class T > Ptr::Shared< T >::~Shared() {
    if ( _p && ( --*_n_ref == 0 ) ) {
        delete _p;
        delete _n_ref;
    }
}

template < class T >
Ptr::Shared< T > &Ptr::Shared< T >::operator=( const Ptr::Shared< T > &p ) {
    if ( _p == p._p ) { return *this; }
    if ( _p && !--*_n_ref ) {
        delete _p;
        delete _n_ref;
    }
    _p     = p._p;
    _n_ref = p._n_ref;
    if ( _p ) { ( *_n_ref )++; }
    return *this;
}

template < class T > T &Ptr::Shared< T >::operator*() { return *_p; }

template < class T > const T &Ptr::Shared< T >::operator*() const {
    return *_p;
}

template < class T > T *Ptr::Shared< T >::operator->() { return _p; }

template < class T > const T *Ptr::Shared< T >::operator->() const {
    return _p;
}

/* -------------------------------------------------------------------------- */
