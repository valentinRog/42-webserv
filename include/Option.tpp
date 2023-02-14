#include "Option.hpp"

/* --------------------------------- Option --------------------------------- */

template < typename T > Option< T >::Option() : _data( 0 ) {}

template < typename T >
Option< T >::Option( const T &value ) : _data( new T( value ) ) {}

template < class T >
Option< T >::Option( const Option< T > &other ) : _data( 0 ) {
    *this = other;
}

template < class T > Option< T >::~Option() {
    if ( is_some() ) { delete _data; }
}

template < class T >
Option< T > &Option< T >::operator=( const Option &other ) {
    if ( this == &other ) { return *this; }
    if ( is_some() ) { delete _data; }
    _data = other.is_some() ? new T( *other._data ) : 0;
    return *this;
}

template < typename T > bool Option< T >::is_some() const { return _data; }

template < typename T > bool Option< T >::is_none() const { return !_data; }

template < typename T > T &Option< T >::unwrap() {
    if ( is_none() ) { throw std::runtime_error( "Option is None" ); }
    return *_data;
}

template < typename T > const T &Option< T >::unwrap() const {
    if ( is_none() ) { throw std::runtime_error( "Option is None" ); }
    return *_data;
}

/* -------------------------------------------------------------------------- */
