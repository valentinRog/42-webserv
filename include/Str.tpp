#include "Str.hpp"

/* ----------------------------------- Str ---------------------------------- */

template < class TI, class TO >
void Str::split( TO &output, const TI &input, const std::string &charset ) {
    std::istringstream ss( input );
    std::string        item;
    while ( std::getline( ss, item, charset[0] ) ) { output.push_back( item ); }
}

/* ------------------------- Str::CaseInsensitiveCmp ------------------------ */

template < class T > std::string Str::from( const T &v ) {
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

/* -------------------------------------------------------------------------- */
