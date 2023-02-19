#include "JSON.hpp"
#include "Option.hpp"
#include "ServerCluster.hpp"
#include "common.h"

static void failed() {
    std::cerr << RED << "invalid configuration file" << RESET << std::endl;
    exit( EXIT_FAILURE );
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
            s.bind( o.unwrap() );
        }
    }
    s.run();
}
