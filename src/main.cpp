#include "JSON.hpp"
#include "ServerCluster.hpp"
#include "common.h"

int main( int argc, char **argv ) {
    if ( argc != 2 ) {
        std::cerr << "usage: ./webserv config.json" << std::endl;
    }
    std::ifstream     f( argv[1] );
    std::stringstream buff;
    buff << f.rdbuf();
    JSON::Array a( JSON::Parse::from_string< JSON::Array >( buff.str() ) );
    for ( JSON::Array::iterator it( a.begin() ); it != a.end(); it++ ) {
        std::cout << *it << std::endl;
    }
    // ServerCluster s;
    // for ( std::vector< JSON::Value * >::const_iterator it( a.get().begin() );
    //       it != ( a.get().end() );
    //       it++ ) {
    //     ServerConf conf( *dynamic_cast< const JSON::Object * >( *it ) );
    //     // s.bind( conf );
    //     std::cout << **it << std::endl;
    // }
    // s.run();
}
