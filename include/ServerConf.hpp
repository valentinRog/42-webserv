#pragma once

#include "JSON.hpp"
#include "common.h"

class ServerConf {
    sockaddr_in _addr;

public:
    ServerConf( const JSON::Object &o );

    const sockaddr_in &get_addr() const;
};
