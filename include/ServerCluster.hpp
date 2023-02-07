#pragma once

#include "EventQueue.hpp"
#include "HTTP.hpp"
#include "ServerConf.hpp"
#include "common.h"

/* ------------------------------ ServerCluster ----------------------------- */

class ServerCluster {
    static const int    _max_events         = 4000;
    static const size_t _buffer_size        = 1024;
    static const time_t _connection_timeout = 30;
    static const time_t _idle_timeout       = 5;

    enum e_config_key { MIME_FILE, SERVERS };
    static const std::string &key_to_string( e_config_key key );
    static const std::map< std::string, e_config_key > &string_to_key();

    /* -------------------- ServerCluster::VirtualHostMapper -------------------- */

    class VirtualHostMapper {
        Ptr::Shared< ServerConf >                          _default;
        std::map< std::string, Ptr::Shared< ServerConf > > _names_map;

    public:
        VirtualHostMapper( const ServerConf &default_conf );

        Ptr::Shared< ServerConf > operator[]( const std::string &s ) const;
        void                      add( const ServerConf &conf );
    };

    /* -------------------------------------------------------------------------- */

    EventQueue                                                    _q;
    std::map< uint16_t, std::map< uint32_t, VirtualHostMapper > > _vh;

    /* ---------------------- ServerCluster::ClientCallback --------------------- */

    class ClientCallback : public CallbackBase {
        int                          _fd;
        ServerCluster &              _server;
        HTTP::Request::DynamicParser _http_parser;
        const VirtualHostMapper &    _vhm;

    public:
        ClientCallback( int                      fd,
                        ServerCluster &          server,
                        const VirtualHostMapper &vhm,
                        time_t                   con_to  = _connection_timeout,
                        time_t                   idle_to = _idle_timeout );
        CallbackBase *clone() const;
        void          handle_read();
        void          handle_write();
        void          handle_timeout();
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
    ServerCluster( const JSON::Object &o );

    void bind( const ServerConf &conf );
    void run();

private:
    void _bind( uint16_t port );
};

/* -------------------------------------------------------------------------- */
