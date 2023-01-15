#pragma once

#include "EventQueue.hpp"
#include "ServerConf.hpp"
#include "VirtualHostMapper.hpp"
#include "common.h"

/* -------------------------------------------------------------------------- */

class ServerCluster {
    static const int    _max_events  = 40;
    static const size_t _buffer_size = 1024;

    struct Addr_Less {
        bool operator()( const sockaddr_in &lhs, const sockaddr_in &rhs ) const;
    };

    EventQueue                                            _q;
    std::set< uint16_t >                                  _ports;
    std::map< sockaddr_in, VirtualHostMapper, Addr_Less > _virtual_hosts;

    class ClientCallback : public CallbackBase {
        int            _fd;
        sockaddr_in    _addr;
        ServerCluster &_server;
        std::string    _s;

    public:
        ClientCallback( int                fd,
                        const sockaddr_in &addr,
                        ServerCluster &    server,
                        time_t             con_to  = 0,
                        time_t             idle_to = 0 );
        CallbackBase *clone() const;
        void          handle_read();
        void          handle_write();
        void          handle_timeout();
    };

    class SocketCallback : public CallbackBase {
        int            _fd;
        sockaddr_in    _addr;
        ServerCluster &_server;

    public:
        SocketCallback( int fd, const sockaddr_in &addr, ServerCluster &s );
        CallbackBase *clone() const;
        void          handle_read();
        void          handle_write();
        void          handle_timeout();
    };

public:
    ServerCluster();

    void bind( const ServerConf &conf );
    void run();

private:
    void _bind( uint16_t port );
};

/* -------------------------------------------------------------------------- */
