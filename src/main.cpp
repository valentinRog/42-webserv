#include "JSON.hpp"
#include "ServerCluster.hpp"
#include "common.h"

int main( int, char ** ) {
    // ServerCluster s;
    // s.bind( 80 );
    // s.bind( 5000 );
    // s.run();
    std::ifstream     f( "compile_commands.json" );
    std::stringstream buff;
    buff << f.rdbuf();
    JSON::Value *v = JSON::Parse::from_string( buff.str() );
    std::cout << *v << std::endl;
    delete v;
}
