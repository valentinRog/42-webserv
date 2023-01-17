#pragma once

#include "JSON.hpp"
#include "common.h"

class ServerConf {
    sockaddr_in             _addr;
    time_t                  _con_to;
    time_t                  _idle_to;
    std::set< std::string > _names;

public:
    class Location {
        std::string             _path;
        std::string             _root;
        std::string             _index;
        std::set< std::string > _method;
        bool                    _dirListing;
        std::string             _redir;
        //std::string new loc ?
    public:
        Location() {}
        Location( std::string                path,
                  std::string                root,
                  std::string                index,
                  std::vector< std::string > method,
                  bool                       dir,
                  std::string                redir );
        ~Location();

        std::string             getPath() { return ( _path ); };
        std::string             getRoot() { return ( _root ); };
        std::string             getIndex() { return ( _index ); };
        std::set< std::string > getMethod() { return ( _method ); };
        bool                    getDirListing() { return ( _dirListing ); };
        std::string             getRedir() { return ( _redir ); };
    };

    ServerConf() {}
    ServerConf( const JSON::Object &o );

    const sockaddr_in &            get_addr() const;
    time_t                         get_con_to() const;
    time_t                         get_idle_to() const;
    const std::set< std::string > &get_names() const;

    //maybe ca marchera pas a partir d'ici
    std::set< std::string > getMethod() const {
        return std::set< std::string >();
    }
    std::string               getErrorPage() const { return "error"; }
    bool                      getDirListing() const { return false; }
    std::string               getIndex() const { return "index"; }
    std::string               getRoot() const { return "root"; }
    std::string               getRedir() const { return "redir"; }
    std::vector< Location * > getLocation() const {
        return std::vector< Location * >( 1, new Location() );
    }

    class ConfigError : public std::exception {
        virtual const char *what() const throw();
    };
};
