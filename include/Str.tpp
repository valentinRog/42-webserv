#include "Str.hpp"

/* -------------------------------------------------------------------------- */

template < class T > std::string Str::from( const T &v ) {
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

/* -------------------------------------------------------------------------- */
