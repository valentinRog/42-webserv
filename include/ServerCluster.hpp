#pragma once

#include "EventQueue.hpp"
#include "HTTP.hpp"
#include "ServerConf.hpp"
#include "VirtualHostMapper.hpp"
#include "common.h"

/* -------------------------------------------------------------------------- */

class ServerCluster {
    static const int    _max_events  = 4000;
    static const size_t _buffer_size = 1024;

    EventQueue                                                    _q;
    std::map< uint16_t, std::map< uint32_t, VirtualHostMapper > > _vh;

    class ClientCallback : public CallbackBase {
        int                      _fd;
        sockaddr_in              _addr;
        ServerCluster &          _server;
        HTTP::DynamicParser      _http_parser;
        const VirtualHostMapper &_vhm;

    public:
        ClientCallback( int                      fd,
                        const sockaddr_in &      addr,
                        ServerCluster &          server,
                        const VirtualHostMapper &vhm,
                        time_t                   con_to  = 0,
                        time_t                   idle_to = 0 );
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
