#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define BUFFER_SIZE 1024

int main() {
    int fd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( fd < 0 ) {
        perror( "socket" );
        return EXIT_FAILURE;
    }
    struct sockaddr_in addr = { 0 };
    addr.sin_family         = AF_INET;
    addr.sin_port           = htons( 80 );
    socklen_t l             = sizeof addr;
    if ( bind( fd, ( struct sockaddr * ) &addr, l ) < 0 ) {
        perror( "bind" );
        return EXIT_FAILURE;
    }
    if ( listen( fd, 1 ) ) {
        perror( "listen" );
        return EXIT_FAILURE;
    }
    std::vector< pollfd > pfds;
    pollfd                pfd = { 0 };
    pfd.fd                    = fd;
    pfd.events                = POLLIN;
    pfds.push_back( pfd );
    while ( ~0 ) {
        poll( pfds.data(), pfds.size(), 1000 );
        for ( std::vector< pollfd >::iterator it = pfds.begin();
              it != pfds.end();
              it++ ) {
            if ( it->revents & POLLIN ) {
                if ( it->fd == fd ) {
                    bzero( &pfd, sizeof( pfd ) );
                    pfd.fd     = accept( fd, ( struct sockaddr     *) &addr, &l );
                    pfd.events = POLLIN;
                    pfds.push_back( pfd );
                    break;
                } else {
                    char   buff[BUFFER_SIZE];
                    size_t n;
                    while ( ( n = read( it->fd, buff, BUFFER_SIZE ) )
                            == BUFFER_SIZE ) {
                        write( STDOUT_FILENO, buff, n );
                    }
                    write( STDOUT_FILENO, buff, n );
                    close( it->fd );
                    pfds.erase( it );
                    break;
                }
            }
        }
    }
}
