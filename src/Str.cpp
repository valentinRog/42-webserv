#include "Str.hpp"

/* -------------------------------------------------------------------------- */

std::string Str::replace_all( const std::string &s,
                              const std::string &from,
                              const std::string &to ) {
    std::string res( s );
    size_t      pos( 0 );
    while ( ( pos = res.find( from, pos ) ) != std::string::npos ) {
        res.replace( pos, from.size(), to );
        pos += to.size();
    }
    return res;
}

std::string Str::erase_all( const std::string &s, const std::string &target ) {
    return Str::replace_all( s, target, "" );
}

bool Str::starts_with( const std::string &s, const std::string &prefix ) {
    return !s.find( prefix );
}

bool Str::ends_with( const std::string &s, const std::string &suffix ) {
    return s.size() >= suffix.size()
           && !s.compare( s.size() - suffix.size(), suffix.size(), suffix );
}

/* -------------------------------------------------------------------------- */
