#include "BiMap.hpp"

/* ---------------------------------- BiMap --------------------------------- */

template < class T1, class T2 >
inline void BiMap< T1, T2 >::insert( std::pair< T1, T2 > v ) {
    if ( _m12.count( v.first ) || _m21.count( v.second ) ) { return; }
    _m12.insert( v );
    _m21.insert( std::make_pair( v.second, v.first ) );
}

template < class T1, class T2 >
inline size_t BiMap< T1, T2 >::count( const T1 &k ) const {
    return _m12.count( k );
}

template < class T1, class T2 >
inline size_t BiMap< T1, T2 >::count( const T2 &k ) const {
    return _m21.count( k );
}

template < class T1, class T2 >
inline const T2 &BiMap< T1, T2 >::at( const T1 &k ) const {
    return _m12.at( k );
}

template < class T1, class T2 >
inline const T1 &BiMap< T1, T2 >::at( const T2 &k ) const {
    return _m21.at( k );
}

/* -------------------------------------------------------------------------- */
