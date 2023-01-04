#include "JSON.hpp"
#include "ServerCluster.hpp"
#include "common.h"

int main( int, char ** ) {
    // ServerCluster s;
    // s.bind( 80 );
    // s.bind( 5000 );
    // s.run();
    // JSON::Value *o( new JSON::Object );
    // JSON::Object *o2 = dynamic_cast< JSON::Object * >( o );
    // o2->add("yo", new JSON::String("aaaaaaaaaaaa"));
    // o2->add("yo2", new JSON::String("aaaaaaaaaaaa"));

    // JSON::Array *a = new JSON::Array();
    // a->add(new JSON::Number(3.14));
    // a->add(new JSON::String("salut"));

    // JSON::Object *o3 = new JSON::Object();
    // o3->add("yo", new JSON::Boolean(true));
    // o2->add("o3", o3);
    // o2->add("arr", a);
    JSON::Object o;
    JSON::String s( "yo" );
    JSON::Array  a;
    a.add( s );
    a.add( s );
    o.add( "1", s );
    o.add( "2", a );
    std::cout << o << std::endl;
}
