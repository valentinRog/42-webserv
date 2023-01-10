#pragma once

#include "JSON.hpp"
#include "common.h"

class ServerConf {
    sockaddr_in _addr;
    time_t      _con_to;
    time_t      _idle_to;

public:
    ServerConf( const JSON::Object &o );

    const sockaddr_in &get_addr() const;

    class ConfigError : public std::exception {
        virtual const char *what() const throw();
    };
};
