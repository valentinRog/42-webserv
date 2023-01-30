#pragma once

#include "common.h"

/* -------------------------------------------------------------------------- */

struct Str {
    static std::string replace_all( const std::string &s,
                                    const std::string &from,
                                    const std::string &to );

    static std::string erase_all( const std::string &s,
                                  const std::string &target );

    static bool starts_with( const std::string &s, const std::string &prefix );

    static bool ends_with( const std::string &s, const std::string &suffix );

    template < typename T > static std::string from( const T &v );
};

/* -------------------------------------------------------------------------- */

#include "Str.tpp"
