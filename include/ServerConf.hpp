#pragma once

#include "JSON.hpp"
#include "Ptr.hpp"
#include "Str.hpp"
#include "Trie.hpp"
#include "common.h"

/* ------------------------------- ServerConf ------------------------------- */

class ServerConf {
public:
    /* ---------------------------- ServerConf::Route --------------------------- */

    class Route {
        std::string              _root;
        std::list< std::string > _index;
        std::set< std::string >  _methods;
        bool                     _autoindex;
        std::string              _redir;

    public:
        Route( const JSON::Object &o );

        const std::string              &root() const;
        const std::list< std::string > &index() const;
        const std::set< std::string >  &methods() const;
        bool                            autoindex() const;
        const std::string              &redir() const;
    };

    /* ------------------------- ServerConf::RouteMapper ------------------------ */

    class RouteMapper {
        std::map< std::string, Route > _routes_table;
        Trie                           _routes;

    public:
        typedef std::map< std::string, Route >::const_iterator const_iterator;

        void         insert( const std::pair< std::string, Route > &v );
        const Route &at( const std::string &s ) const;
        std::map< std::string, Route >::size_type
                    count( const std::string &s ) const;
        std::string suffix( const std::string &s ) const;
    };

    /* ------------------ ServerConf::RouteMapper::ConfigError ------------------ */

    class ConfigError : public std::exception {
        virtual const char *what() const throw();
    };

    /* -------------------------------------------------------------------------- */

private:
    sockaddr_in                                         _addr;
    std::set< std::string >                             _names;
    RouteMapper                                         _route_mapper;
    Ptr::Shared< std::map< std::string, std::string > > _mime;

public:
    ServerConf( const JSON::Object                                 &o,
                Ptr::Shared< std::map< std::string, std::string > > mime );

    const sockaddr_in                          &addr() const;
    const std::set< std::string >              &names() const;
    const RouteMapper                          &route_mapper() const;
    const std::map< std::string, std::string > &mime() const;
};

/* -------------------------------------------------------------------------- */
