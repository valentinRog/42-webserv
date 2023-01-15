#pragma once

#include "ServerConf.hpp"
#include "common.h"

class VirtualHostMapper {
    std::vector< ServerConf >             _conf;
    std::map< std::string, ServerConf * > _names_map;

public:
    VirtualHostMapper( const ServerConf &default_conf );
    VirtualHostMapper( const VirtualHostMapper &other );
    VirtualHostMapper &operator=( const VirtualHostMapper &other );

    const ServerConf &get_default() const;
    const ServerConf &operator[]( const std::string &s ) const;
    void              add( const ServerConf &conf );
};
