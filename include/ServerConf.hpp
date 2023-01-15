#pragma once

#include "JSON.hpp"
#include "common.h"

class ServerConf {
    sockaddr_in             _addr;
    time_t                  _con_to;
    time_t                  _idle_to;
    std::set< std::string > _names;

public:
    ServerConf( const JSON::Object &o );

    const sockaddr_in &            get_addr() const;
    time_t                         get_con_to() const;
    time_t                         get_idle_to() const;
    const std::set< std::string > &get_names() const;

    class ConfigError : public std::exception {
        virtual const char *what() const throw();
    };
};
