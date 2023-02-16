#include "Str.hpp"

/* ----------------------------------- Str ---------------------------------- */

template < typename T >
void Str::split( T                 &output,
                 const std::string &input,
                 const std::string &charset ) {
    std::istringstream ss( input );
    std::string        item;
    while ( std::getline( ss, item, charset[0] ) ) { output.push_back( item ); }
}

template < typename T >
size_t
Str::cat_until( std::string &s, T first, T last, const std::string &end ) {
    size_t i( 0 );
    for ( ; !ends_with( s, end ) && first != last; first++, i++ ) {
        s += *first;
    }
    return i;
}

/* ------------------------- Str::CaseInsensitiveCmp ------------------------ */

template < typename T > std::string Str::from( const T &v ) {
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

/* -------------------------------------------------------------------------- */
