#pragma once

#include "common.h"

/* ---------------------------------- BiMap --------------------------------- */

template < typename T1, typename T2 > class BiMap {
    std::map< T1, T2 > _m12;
    std::map< T2, T1 > _m21;

public:
    void      insert( std::pair< T1, T2 > v );
    size_t    count( const T1 &k ) const;
    size_t    count( const T2 &k ) const;
    const T2 &at( const T1 &k ) const;
    const T1 &at( const T2 &k ) const;
};

/* -------------------------------------------------------------------------- */

#include "BiMap.tpp"
