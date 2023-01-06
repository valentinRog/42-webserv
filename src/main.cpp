#include "JSON.hpp"
#include "ServerCluster.hpp"
#include "common.h"

int main( int, char ** ) {
    // ServerCluster s;
    // s.bind( 80 );
    // s.bind( 5000 );
    // s.run();
    std::ifstream     f( "test.json" );
    std::stringstream buff;
    buff << f.rdbuf();
    JSON::Object o = JSON::Parse::from_string( buff.str() );
    std::cout << o << std::endl;
}
