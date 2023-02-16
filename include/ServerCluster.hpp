#pragma once

#include "BiMap.hpp"
#include "EventQueue.hpp"
#include "HTTP.hpp"
#include "RequestHandler.hpp"
#include "ServerConf.hpp"
#include "common.h"

/* ------------------------------ ServerCluster ----------------------------- */

class ServerCluster {
    static const int    _max_events         = 4000;
    static const size_t _buffer_size        = 8192;
    static const time_t _connection_timeout = 30;
    static const time_t _idle_timeout       = 5;

    /* -------------------- ServerCluster::VirtualHostMapper -------------------- */

    class VirtualHostMapper {
        Ptr::Shared< ServerConf >                          _default;
        std::map< std::string, Ptr::Shared< ServerConf > > _names_map;

    public:
        VirtualHostMapper( const ServerConf &default_conf );

        Ptr::Shared< ServerConf > get_default() const;
        Ptr::Shared< ServerConf > operator[]( const std::string &s ) const;
        void                      add( const ServerConf &conf );
    };

    /* -------------------------------------------------------------------------- */

    EventQueue                                                    _q;
    std::map< uint16_t, std::map< uint32_t, VirtualHostMapper > > _vh;

    /* ---------------------- ServerCluster::ClientCallback --------------------- */

    class ClientCallback : public CallbackBase {
        int                                _fd;
        const VirtualHostMapper           &_vhm;
        HTTP::Request::DynamicParser       _http_parser;
        std::string                        _accu;
        Option< HTTP::ContentAccumulator > _content;
        std::string                        _raw_request_line;
        std::string                        _raw_header_line;

    public:
        ClientCallback( int                      fd,
                        const VirtualHostMapper &vhm,
                        time_t                   con_to  = _connection_timeout,
                        time_t                   idle_to = _idle_timeout );
        CallbackBase *clone() const;
        void          handle_read();
        void          handle_write();
        void          handle_timeout();

    private:
        void _log_write_failure( HTTP::Response::e_error_code code ) const;
        void _log_write_response( HTTP::Response::e_error_code code ) const;
    };

    /* ---------------------- ServerCluster::SocketCallback --------------------- */

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

    /* -------------------------------------------------------------------------- */

public:
    ServerCluster();

    void bind( const ServerConf &conf );
    void run();

private:
    void _bind( uint16_t port );
};

/* -------------------------------------------------------------------------- */
