#include "JSON.hpp"
#include "ServerCluster.hpp"
#include "common.h"

int main( int argc, char **argv ) {
    if ( argc != 2 ) {
        std::cerr << "usage: ./webserv config.json" << std::endl;
        return EXIT_FAILURE;
    }
    ServerCluster s;
    {
        JSON::Array a;
        try {
            a = JSON::Parse::from_file( argv[1] ).unwrap< JSON::Array >();
            for ( JSON::Array::const_iterator it = a.begin(); it != a.end();
                  it++ ) {
                it->unwrap< JSON::Object >();
            }
        } catch ( const JSON::Parse::ParsingError &e ) {
            std::cerr << RED << e.what() << RESET << std::endl;
            return EXIT_FAILURE;
        } catch ( const std::bad_cast & ) {
            std::cerr << RED << "Bad JSON schema\n" << RESET;
            return EXIT_FAILURE;
        }
        for ( JSON::Array::const_iterator it = a.begin(); it != a.end();
              it++ ) {
            JSON::Object o( it->unwrap< JSON::Object >() );
            try {
                s.bind( o );
            } catch ( const ServerConf::ConfigError &e ) {
                std::cerr << RED << e.what() << RESET << std::endl;
                return EXIT_FAILURE;
            }
        }
    }
    s.run();
}
