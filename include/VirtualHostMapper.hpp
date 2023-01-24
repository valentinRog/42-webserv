#pragma once

#include "ServerConf.hpp"
#include "Wrapper.hpp"
#include "common.h"

/* -------------------------------------------------------------------------- */

class VirtualHostMapper {
    std::list< ServerConf >                                          _conf;
    std::map< std::string, std::list< ServerConf >::const_iterator > _names_map;

public:
    VirtualHostMapper( const ServerConf &default_conf );
    VirtualHostMapper( const VirtualHostMapper &other );
    VirtualHostMapper &operator=( const VirtualHostMapper &other );

    const ServerConf &get_default() const;
    const ServerConf &operator[]( const std::string &s ) const;
    void              add( const ServerConf &conf );
};

/* -------------------------------------------------------------------------- */
