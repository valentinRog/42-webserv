#pragma once

#include "common.h"

#define BUFFER_SIZE 1024

class Server {
    int           _fd;
    sockaddr_in   _addr;
    int           _kq;
    struct kevent _change_event[4];
    struct kevent _event[4];

public:
    Server();

    void run();
};
