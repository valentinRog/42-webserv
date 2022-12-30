#include "common.h"
#include "ServerCluster.hpp"

int main( int, char ** ) {
    ServerCluster s;
    s.bind(80);
    s.bind(5000);
    s.run();

}
