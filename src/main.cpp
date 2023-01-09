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
    JSON::Array a
        = JSON::Parse::from_string( buff.str() ).unwrap< JSON::Array >();
    for ( JSON::Array::iterator it( a.begin() ); it != a.end(); it++ ) {
        std::cout << it->unwrap() << std::endl;
    }
    JSON::Array x;
    x.push_back( JSON::Number( 8 ) );
    double n = x.front().unwrap< JSON::Number >();
    std::cout << n << std::endl;
    x[0] = JSON::Number( 42 );
    std::cout << x << std::endl;
    JSON::Wrapper w;
    std::cout << w.unwrap() << std::endl;
    w = JSON::Number( 8 );
    std::cout << w.unwrap() << std::endl;
    w                                 = JSON::Object();
    w.unwrap< JSON::Object >()["key"] = JSON::Boolean( true );
    std::cout << w.unwrap() << std::endl;
}
