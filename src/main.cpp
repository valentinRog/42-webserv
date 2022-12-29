#include "common.h"

int main( int, char ** ) {
    std::cout << "this is a non-blocking webserver" << std::endl;
    Server s;
    s.bind(80);
    s.run();
}
