#include "Str.hpp"

/* ----------------------------------- Str ---------------------------------- */

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

std::string Str::to_lower( const std::string &s ) {
    std::string res( s );
    std::transform( res.begin(), res.end(), res.begin(), ::tolower );
    return res;
}

std::string Str::to_upper( const std::string &s ) {
    std::string res( s );
    std::transform( res.begin(), res.end(), res.begin(), ::toupper );
    return res;
}

std::string Str::trim_left( const std::string &s, const std::string &charset ) {
    size_t n( s.find_first_not_of( charset ) );
    return n == std::string::npos ? "" : s.substr( n, s.size() - n );
}

std::string Str::trim_right( const std::string &s,
                             const std::string &charset ) {
    size_t n( s.find_last_not_of( charset ) );
    return n == std::string::npos ? "" : s.substr( 0, n + 1 );
}

std::string Str::trim( const std::string &s, const std::string &charset ) {
    size_t first = s.find_first_not_of( charset );
    size_t last  = s.find_last_not_of( charset );
    return first == std::string::npos ? ""
                                      : s.substr( first, last - first + 1 );
}

char *Str::dup( const std::string &s ) {
    char *res = new char[s.size() + 1];
    ::strcpy( res, s.c_str() );
    return res;
}

/* ------------------------- Str::CaseInsensitiveCmp ------------------------ */

bool Str::CaseInsensitiveCmp::operator()( const std::string &s1,
                                          const std::string &s2 ) const {
    return to_lower( s1 ) < to_lower( s2 );
}

/* -------------------------------------------------------------------------- */
