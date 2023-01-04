#pragma once

#include "EventQueue.hpp"
#include "ServerConf.hpp"
#include "common.h"

/* -------------------------------------------------------------------------- */

class ServerCluster {
    EventQueue          _q;
    static const int    _max_events  = 40;
    static const size_t _buffer_size = 1024;

    class ClientCallback : public CallbackBase {
        int         _fd;
        EventQueue &_q;
        std::string _s;

    public:
        ClientCallback( int fd, EventQueue &q );
        CallbackBase *clone() const;
        void          handle_read();
        void          handle_write();
        void          handle_timeout();
    };

    class SocketCallback : public CallbackBase {
        int         _fd;
        ServerConf  _conf;
        EventQueue &_q;

    public:
        SocketCallback( int fd, ServerConf conf, EventQueue &q );
        CallbackBase *clone() const;
        void          handle_read();
        void          handle_write();
        void          handle_timeout();
    };

public:
    ServerCluster();

    void bind( uint16_t port );
    void run();
};

/* -------------------------------------------------------------------------- */
