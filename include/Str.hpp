#pragma once

#include "common.h"

/* ----------------------------------- Str ---------------------------------- */

struct Str {
    static std::string replace_all( const std::string &s,
                                    const std::string &from,
                                    const std::string &to );

    static std::string erase_all( const std::string &s,
                                  const std::string &target );

    static bool starts_with( const std::string &s, const std::string &prefix );

    static bool ends_with( const std::string &s, const std::string &suffix );

    template < typename T > static std::string from( const T &v );

    static std::string to_lower( const std::string &s );

    static std::string to_upper( const std::string &s );

    static std::string trim_left(const std::string &s, const std::string &charset);

    static std::string trim_right(const std::string &s, const std::string &charset);

    static std::string trim(const std::string &s, const std::string &charset);

    static char *dup(const std::string &s);

    template<typename TI, typename TO>
    static void split(TO& output, const TI& input, const std::string& charset);

    /* ------------------------- Str::CaseInsensitiveCmp ------------------------ */

    struct CaseInsensitiveCmp {
        bool operator()( const std::string &s1, const std::string &s2 ) const;
    };

    /* -------------------------------------------------------------------------- */
};

/* -------------------------------------------------------------------------- */

#include "Str.tpp"
