#include "Ptr.hpp"

/* -------------------------------------------------------------------------- */

template < class T > Ptr::unique< T >::unique() : _p( 0 ) {}

template < class T > Ptr::unique< T >::unique( T *p ) : _p( p ) {}

template < class T > Ptr::unique< T >::~unique() { delete _p; }

template < class T > T *Ptr::unique< T >::release() {
    T *tmp( _p );
    _p = 0;
    return tmp;
}

template < class T > void Ptr::unique< T >::reset( T *p ) {
    if ( _p != p ) {
        delete _p;
        _p = p;
    }
}
template < class T > T &Ptr::unique< T >::operator*() { return *_p; }

template < class T > const T &Ptr::unique< T >::operator*() const {
    return *_p;
}

template < class T > T *Ptr::unique< T >::operator->() { return _p; }

template < class T > const T *Ptr::unique< T >::operator->() const {
    return _p;
}
/* -------------------------------------------------------------------------- */

template < class T > Ptr::shared< T >::shared( T *p ) : _p( p ) {
    if ( _p ) {
        _n_ref = new int( 1 );
    } else {
        _n_ref = 0;
    }
}

template < class T >
Ptr::shared< T >::shared( const Ptr::shared< T > &p )
    : _p( p._p ),
      _n_ref( p._n_ref ) {
    if ( _p ) { ( *_n_ref )++; }
}

template < class T > Ptr::shared< T >::~shared() {
    if ( _p && ( --*_n_ref == 0 ) ) {
        delete _p;
        delete _n_ref;
    }
}

template < class T >
Ptr::shared< T > &Ptr::shared< T >::operator=( const Ptr::shared< T > &p ) {
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

template < class T > T &Ptr::shared< T >::operator*() { return *_p; }

template < class T > const T &Ptr::shared< T >::operator*() const {
    return *_p;
}

template < class T > T *Ptr::shared< T >::operator->() { return _p; }

template < class T > const T *Ptr::shared< T >::operator->() const {
    return _p;
}

/* -------------------------------------------------------------------------- */
