#pragma once

#include "common.h"

class ServerConf {
    sockaddr_in _addr;

public:
    ServerConf( uint16_t port );

    sockaddr_in &get_addr();
};