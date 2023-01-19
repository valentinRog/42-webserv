#pragma once

#include "JSON.hpp"
#include "common.h"

/* -------------------------------------------------------------------------- */

struct ServerConf {
    struct Location {
        std::string             path;
        std::string             root;
        std::string             index;
        std::set< std::string > methods;
        bool                    autoindex;
        std::string             redir;

        Location();
        Location( const JSON::Object &o );
    };

    sockaddr_in             addr;
    time_t                  con_to;
    time_t                  idle_to;
    std::set< std::string > names;
    std::string             error_page;
    std::string             root;
    std::vector< Location > locations;

    ServerConf();
    ServerConf( const JSON::Object &o );

    class ConfigError : public std::exception {
        virtual const char *what() const throw();
    };
};

/* -------------------------------------------------------------------------- */
