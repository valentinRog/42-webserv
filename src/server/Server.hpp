#pragma once

#include "common.h"

#define BUFFER_SIZE 1024

class Server {
    int           _fd;
    sockaddr_in   _addr;
    int           _kq;
    struct kevent _change_event[40];
    struct kevent _event[40];

public:
    Server();
    
    void bind(uint16_t port);
    void run();
};
