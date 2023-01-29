#pragma once

#include "JSON.hpp"
#include "Trie.hpp"
#include "common.h"

/* -------------------------------------------------------------------------- */

struct ServerConf {
    struct Route {
        std::string              path;
        std::string              root;
        std::list< std::string > index;
        std::set< std::string >  methods;
        bool                     autoindex;
        std::string              redir;

        Route() {}
        Route( const JSON::Object &o );
    };

    class RouteMapper {
        std::map< std::string, Route > _routes_table;
        Trie                           _routes;

    public:
        typedef std::map< std::string, Route >::const_iterator const_iterator;

        Route &operator[]( const std::string &s ) {
            _routes.insert( s );
            return _routes_table[_routes.lower_bound( s )];
        }
        const Route &at( const std::string &s ) const {
            return _routes_table.at( _routes.lower_bound( s ) );
        }
        const_iterator find( const std::string &s ) const {
            return _routes_table.find( _routes.lower_bound( s ) );
        }
        std::string route_name( const std::string &s ) const {
            return _routes.lower_bound( s );
        }
        std::string suffix( const std::string &s ) const {
            std::string::size_type l( _routes.lower_bound( s ).size() );
            return s.substr( l, s.size() - l );
        }
    };

    sockaddr_in             addr;
    std::set< std::string > names;
    std::string             error_page;
    RouteMapper             route_mapper;

    ServerConf() {}
    ServerConf( const JSON::Object &o );

    class ConfigError : public std::exception {
        virtual const char *what() const throw();
    };
};

/* -------------------------------------------------------------------------- */
