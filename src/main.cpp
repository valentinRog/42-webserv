#include "JSON.hpp"
#include "ServerCluster.hpp"
#include "Wrapper.hpp"
#include "common.h"

int main( int argc, char **argv ) {
    if ( argc != 2 ) {
        std::cerr << "usage: ./webserv config.json" << std::endl;
    }
    ServerCluster s;
    {
        std::ifstream     f( argv[1] );
        std::stringstream buff;
        buff << f.rdbuf();
        JSON::Array a(
            JSON::Parse::from_string( buff.str() ).unwrap< JSON::Array >() );
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
