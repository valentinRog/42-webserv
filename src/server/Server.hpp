#pragma once

#include "common.h"
#include <map>

#define BUFFER_SIZE 1024

class Server {
    std::map< int, ServerConf > _servers;
    int                         _kq;
    struct kevent               _change_event[40];
    struct kevent               _event[40];

public:
    Server();

    void bind( uint16_t port );
    void run();
};
