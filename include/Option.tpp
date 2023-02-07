#include "Option.hpp"

/* --------------------------------- Option --------------------------------- */

template < class T > inline Option< T >::Option() : _is_some( false ) {}

template < class T >
inline Option< T >::Option( const T &value ) : _is_some( true ) {
    _data = value;
}

template < class T >
inline Option< T > &Option< T >::operator=( const T &value ) {
    _data    = value;
    _is_some = true;
    return *this;
}

template < class T > inline bool Option< T >::is_some() const {
    return _is_some;
}

template < class T > inline bool Option< T >::is_none() const {
    return !_is_some;
}

template < class T > T &Option< T >::unwrap() {
    if ( !_is_some ) { throw std::runtime_error( "Option is None" ); }
    return _data;
}

template < class T > inline const T &Option< T >::unwrap() const {
    if ( !_is_some ) { throw std::runtime_error( "Option is None" ); }
    return _data;
}

/* -------------------------------------------------------------------------- */
