#pragma once

#include "BiMap.hpp"
#include "EventQueue.hpp"
#include "HTTP.hpp"
#include "RequestHandler.hpp"
#include "ServerConf.hpp"
#include "common.h"

/* ------------------------------ ServerCluster ----------------------------- */

class ServerCluster {
    static const int    _MAX_EVENTS         = 4000;
    static const size_t _BUFFER_SIZE        = 8192;
    static const time_t _CONNECTION_TIMEOUT = 30;
    static const time_t _IDLE_TIMEOUT       = 5;

    /* -------------------- ServerCluster::VirtualHostMapper -------------------- */

    class VirtualHostMapper {
        Ptr::Shared< ServerConf >                          _default;
        std::map< std::string, Ptr::Shared< ServerConf > > _names_map;

    public:
        VirtualHostMapper( const ServerConf &default_conf );

        Ptr::Shared< ServerConf > get_default() const;
        Ptr::Shared< ServerConf > operator[]( const std::string &s ) const;
        size_t                    add( const ServerConf &conf );
    };

    /* -------------------------------------------------------------------------- */

    EventQueue                                                    _q;
    std::map< uint16_t, std::map< uint32_t, VirtualHostMapper > > _vh;

    /* ---------------------- ServerCluster::ClientCallback --------------------- */

    class ClientCallback : public CallbackBase {
        enum e_step { HEADER, CONTENT, BODY, DONE, FAILED };

        int                        _fd;
        const VirtualHostMapper &  _vhm;
        std::string                _raw;
        std::string                _raw_request;
        std::string                _raw_header;
        Ptr::Shared< std::string > _raw_content;
        Option< PolymorphicWrapper< HTTP::ContentAccumulatorBase > > _accu;
        Option< HTTP::Request >                                      _request;
        Option< HTTP::Response::e_error_code >                       _error;

    public:
        ClientCallback( int                      fd,
                        const VirtualHostMapper &vhm,
                        time_t                   con_to  = _CONNECTION_TIMEOUT,
                        time_t                   idle_to = _IDLE_TIMEOUT );
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

    bool bind( const ServerConf &conf );
    void run();

private:
    bool _bind( uint16_t port );
};

/* -------------------------------------------------------------------------- */
