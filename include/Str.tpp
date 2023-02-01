#include "Str.hpp"

/* ------------------------- Str::CaseInsensitiveCmp ------------------------ */

template < class T > std::string Str::from( const T &v ) {
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

/* -------------------------------------------------------------------------- */
