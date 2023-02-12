#pragma once

#include "common.h"

template < typename T1, typename T2 > class BiMap {
    std::map< T1, T2 > _m12;
    std::map< T2, T1 > _m21;

public:
    void insert( std::pair< T1, T2 > v ) {
        _m12.insert( v );
        _m21.insert( std::make_pair( v.second, v.first ) );
    }
    size_t    count( const T1 &k ) const { return _m12.count( k ); }
    size_t    count( const T2 &k ) const { return _m21.count( k ); }
    const T2 &at( const T1 &k ) const { return _m12.at( k ); }
    const T1 &at( const T2 &k ) const { return _m21.at( k ); }
};
