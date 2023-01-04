#include "JSON.hpp"
#include "ServerCluster.hpp"
#include "common.h"

int main( int, char ** ) {
    ServerCluster s;
    s.bind( 80 );
    s.bind( 5000 );
    s.run();
    // JSON::Value *o( new JSON::Object );
    // JSON::Object *o2 = dynamic_cast< JSON::Object * >( o );
    // o2->add("yo", new JSON::String("aaaaaaaaaaaa"));
    // o2->add("yo2", new JSON::String("aaaaaaaaaaaa"));
    // std::cout << *o << std::endl;
}
