#include "JSON.hpp"
#include "Option.hpp"
#include "ServerCluster.hpp"
#include "common.h"

static void failed() {
    std::cerr << RED << "invalid configuration" << RESET << std::endl;
    exit( EXIT_FAILURE );
}

static void log_bind( const JSON::Object &o, bool success ) {
    JSON::Array                  names;
    JSON::Object::const_iterator it = o.find( "names" );
    if ( it != o.end() ) { names = *it->second.dycast< JSON::Array >(); }
    std::cout << YELLOW << "binding " << RESET << o.at( "listen" )->stringify()
              << " with names: " << names.stringify() << BLUE " -> ";
    if ( success ) {
        std::cout << GREEN << "success" << RESET << std::endl;
    } else {
        std::cout << RED << "failure" << RESET << std::endl;
    }
}

int main( int argc, char **argv ) {
    if ( argc != 2 ) {
        std::cerr << "usage: ./webserv config.json" << std::endl;
        return EXIT_FAILURE;
    }
    ServerCluster s;
    {
        Option< JSON::Wrapper > o = JSON::Parse::from_file( argv[1] );
        if ( o.is_none() ) { failed(); }
        const JSON::Array *p = o.unwrap().dycast< JSON::Array >();
        if ( !p ) { failed(); }
        for ( JSON::Array::const_iterator it = p->begin(); it != p->end();
              it++ ) {
            if ( !it->dycast< JSON::Object >() ) { failed(); }
            const JSON::Object *p = it->dycast< JSON::Object >();
            if ( !p ) { failed(); }
            Option< ServerConf > o = ServerConf::from_json( *p );
            if ( o.is_none() ) { failed(); }
            log_bind(*p, !s.bind( o.unwrap() ));
        }
    }
    s.run();
}
