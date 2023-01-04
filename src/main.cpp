#include "JSON.hpp"
#include "ServerCluster.hpp"
#include "common.h"

int main( int, char ** ) {
    // ServerCluster s;
    // s.bind( 80 );
    // s.bind( 5000 );
    // s.run();
    JSON::Object o;
    JSON::Array  a;
    JSON::Object o2;
    o2.add( "key", JSON::String( "value" ) );
    a.add( JSON::Null() );
    a.add( JSON::String( "yo" ) );
    a.add( JSON::Number( 3.14 ) );
    a.add( JSON::Boolean( true ) );
    a.add( o2 );
    o.add( "1", JSON::Null() );
    o.add( "2", a );
    std::cout << o << std::endl;
}
