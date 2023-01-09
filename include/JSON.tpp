#include "JSON.hpp"

/* -------------------------------------------------------------------------- */

template < typename T > T &JSON::Wrapper::unwrap() {
    return dynamic_cast< T & >( *_v );
}

template < typename T > const T &JSON::Wrapper::unwrap() const {
    return dynamic_cast< const T & >( *_v );
}

/* -------------------------------------------------------------------------- */
