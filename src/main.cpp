#include "JSON.hpp"
#include "ServerCluster.hpp"
#include "Wrapper.hpp"
#include "common.h"

int main( int argc, char **argv ) {
    if ( argc != 2 ) {
        std::cerr << "usage: ./webserv config.json" << std::endl;
        return EXIT_FAILURE;
    }
    HTTP::Values();
    ServerCluster s;
    
    {
        JSON::Array a(
            JSON::Parse::from_file( argv[1] ).unwrap< JSON::Array & >() );
        std::vector< ServerConf > v;
        for ( JSON::Array::const_iterator it( a.begin() ); it != a.end();
              it++ ) {
            v.push_back( it->unwrap< JSON::Object >() );
        }
        for ( std::vector< ServerConf >::const_iterator it = v.begin();
              it != v.end();
              it++ ) {
            s.bind( *it );
        }
    }
    s.run();
}
