#include "common.h"

int main( int argc, char **) {
    if ( argc != 2 ) {
        std::cerr << "ntm" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "this is a non-blocking webserver" << std::endl;
    Server s;
    s.run();
}
