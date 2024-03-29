#pragma once

#include "HTTP.hpp"
#include "JSON.hpp"
#include "Option.hpp"
#include "Ptr.hpp"
#include "Str.hpp"
#include "Trie.hpp"
#include "common.h"

/* ------------------------------- ServerConf ------------------------------- */

class ServerConf {
public:
    /* ---------------------------- ServerConf::Route --------------------------- */

    class Route {
        enum e_config_key {
            ROOT,
            INDEX,
            METHODS,
            CGI,
            AUTOINDEX,
            REDIR,
            HANDLER
        };
        static const BiMap< e_config_key, std::string > &key_to_string();

        Route();

        std::string                          _root;
        std::string                          _index;
        std::set< std::string >              _methods;
        bool                                 _autoindex;
        std::string                          _redir;
        std::map< std::string, std::string > _cgis;
        std::string                          _handler;

    public:
        static Option< Route > from_json( const JSON::Object &o );

        const std::string &                         root() const;
        const std::string &                         index() const;
        const std::set< std::string > &             methods() const;
        bool                                        autoindex() const;
        const std::string &                         redir() const;
        const std::map< std::string, std::string > &cgis() const;
        const std::string &                         handler() const;
    };

    /* ------------------------- ServerConf::RouteMapper ------------------------ */

    class RouteMapper {
        std::map< std::string, Route > _routes_table;
        Trie                           _routes;

    public:
        typedef std::map< std::string, Route >::const_iterator const_iterator;

        void         insert( const std::pair< std::string, Route > &v );
        const Route &at( const std::string &s ) const;
        size_t       count( const std::string &s ) const;
        std::string  route_name( const std::string &s ) const;
        std::string  suffix( const std::string &s ) const;
    };

    /* -------------------------------------------------------------------------- */

private:
    ServerConf();

    sockaddr_in                                           _addr;
    std::set< std::string >                               _names;
    RouteMapper                                           _route_mapper;
    std::map< HTTP::Response::e_error_code, std::string > _code_to_error_page;
    size_t                                                _client_max_body_size;

public:
    enum e_config_key {
        LISTEN,
        SERVER_NAMES,
        CLIENT_MAX_BODY_SIZE,
        ROUTES,
        ERROR_PAGES
    };
    static const BiMap< e_config_key, std::string > &key_to_string();

    static Option< ServerConf > from_json( const JSON::Object &o );

    const sockaddr_in &            addr() const;
    const std::set< std::string > &names() const;
    const RouteMapper &            route_mapper() const;
    const std::map< HTTP::Response::e_error_code, std::string > &
           code_to_error_page() const;
    size_t client_max_body_size() const;
};

/* -------------------------------------------------------------------------- */
