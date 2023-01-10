#include "JSON.hpp"
#include "ServerCluster.hpp"
#include "common.h"

int main( int argc, char **argv ) {
    if ( argc != 2 ) {
        std::cerr << "usage: ./webserv config.json" << std::endl;
    }
    ServerCluster     s;
    std::ifstream     f( argv[1] );
    std::stringstream buff;
    buff << f.rdbuf();
    JSON::Array a(
        JSON::Parse::from_string( buff.str() ).unwrap< JSON::Array >() );
    for ( JSON::Array::const_iterator it( a.begin() ); it != a.end(); it++ ) {
        ServerConf conf( it->unwrap< JSON::Object >() );
        s.bind( conf );
    }
    s.run();
}
