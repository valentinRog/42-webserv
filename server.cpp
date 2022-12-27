#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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
    int                kq = kqueue();
    struct kevent      change_event[4], event[4];
    struct sockaddr_in serv_addr, client_addr;
    EV_SET( change_event, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0 );
    if ( kevent( kq, change_event, 1, NULL, 0, NULL ) < 0 ) {
        perror( "kqueue" );
        return EXIT_FAILURE;
    }
    while ( ~0 ) {
        int new_events = kevent( kq, NULL, 0, event, 1, NULL );
        for ( int i = 0; new_events > i; i++ ) {
            int efd = event[i].ident;
            if ( event[i].flags & EV_EOF ) {
                close( efd );
            } else if ( efd == fd ) {
                int cfd = accept( fd, ( struct sockaddr * ) &addr, &l );
                EV_SET( change_event, cfd, EVFILT_READ, EV_ADD, 0, 0, NULL );
                kevent( kq, change_event, 1, NULL, 0, NULL );
            } else if ( event[i].filter & EVFILT_READ ) {
                char   buff[BUFFER_SIZE];
                size_t n;
                while ( ( n = read( efd, buff, BUFFER_SIZE ) )
                        == BUFFER_SIZE ) {
                    write( STDOUT_FILENO, buff, n );
                }
                write( STDOUT_FILENO, buff, n );
                printf( "\n\n" );
                fflush( stdin );
            }
        }
    }
}
