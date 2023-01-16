#include "Wrapper.hpp"

/* -------------------------------------------------------------------------- */

template < typename T >
PolymorphicWrapper< T >::PolymorphicWrapper( const T &v ) : _p( v.clone() ) {}

template < typename T >
PolymorphicWrapper< T >::PolymorphicWrapper( const PolymorphicWrapper &other )
    : _p( other._p->clone() ) {}

template < typename T > PolymorphicWrapper< T >::~PolymorphicWrapper() {
    delete _p;
}

template < class T >
PolymorphicWrapper< T > &
PolymorphicWrapper< T >::operator=( const PolymorphicWrapper< T > &other ) {
    T *tmp( other._p->clone() );
    delete _p;
    _p = tmp;
    return *this;
}

template < typename T > T &PolymorphicWrapper< T >::operator*() { return *_p; }

template < typename T > const T &PolymorphicWrapper< T >::operator*() const {
    return *_p;
}

template < typename T > T *PolymorphicWrapper< T >::operator->() { return _p; }

template < typename T > const T *PolymorphicWrapper< T >::operator->() const {
    return _p;
}

template < typename T >
template < typename U >
U &PolymorphicWrapper< T >::unwrap() {
    return dynamic_cast< U & >( *_p );
}

template < typename T >
template < typename U >
const U &PolymorphicWrapper< T >::unwrap() const {
    return dynamic_cast< const U & >( *_p );
}

/* -------------------------------------------------------------------------- */
