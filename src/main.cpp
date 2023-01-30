#include "JSON.hpp"
#include "ServerCluster.hpp"
#include "common.h"

int main( int argc, char **argv ) {
    if ( argc != 2 ) {
        std::cerr << "usage: ./webserv config.json" << std::endl;
        return EXIT_FAILURE;
    }
    ServerCluster( JSON::Parse::from_file( argv[1] ).unwrap< JSON::Object >() )
        .run();
}
