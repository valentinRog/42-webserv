#pragma once

#include "EventQueue.hpp"
#include "ServerConf.hpp"
#include "common.h"

#define BUFFER_SIZE 1024
#define MAX_EVENTS 40

class ServerCluster {
    EventQueue _q;

    class ClientCallback {
        int         _fd;
        EventQueue &_q;

    public:
        ClientCallback( int fd, EventQueue &q );
        void operator()();
    };

    class SocketCallback {
        int         _fd;
        ServerConf  _conf;
        EventQueue &_q;

    public:
        SocketCallback( int fd, ServerConf conf, EventQueue &q );
        void operator()();
    };

public:
    ServerCluster();

    void bind( uint16_t port );
    void run();
};
