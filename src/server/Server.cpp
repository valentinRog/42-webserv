#include "Server.hpp"

Server::Server() {  _kq = kqueue();}

void Server::bind( uint16_t port ) {
    _fd = socket( AF_INET, SOCK_STREAM, 0 );
    bzero( &_addr, sizeof _addr );
    _addr.sin_family = AF_INET;
    _addr.sin_port   = htons( port );
    if ( ::bind( _fd, ( sockaddr * ) &_addr, sizeof _addr ) < 0 ) {
        perror( "bind" );
        exit( EXIT_FAILURE );
    }
    listen( _fd, 100 );
    EV_SET( _change_event, _fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0 );
    kevent( _kq, _change_event, 1, NULL, 0, NULL );
}

void Server::run() {
    while ( ~0 ) {
        int ne = kevent( _kq, NULL, 0, _event, 1, NULL );
        for ( int i = 0; i < ne; i++ ) {
            int efd = _event[i].ident;
            if ( efd == _fd ) {
                socklen_t l   = sizeof( sockaddr_in );
                int       cfd = accept( _fd, ( struct sockaddr       *) &_addr, &l );
                EV_SET( _change_event, cfd, EVFILT_READ, EV_ADD, 0, 0, NULL );
                kevent( _kq, _change_event, 1, NULL, 0, NULL );
                std::cout << "accepted" << std::endl;
            } else if ( _event[i].filter & EVFILT_READ ) {
                char   buff[BUFFER_SIZE];
                size_t n = read( efd, buff, BUFFER_SIZE );
                write( STDOUT_FILENO, buff, n );
                fflush( stdin );
                if ( n >= 4 && !std::strncmp( buff + n - 4, "\r\n\r\n", 4 ) ) {
                    write( efd, "yo", 2 );
                    close( efd );
                }
            }
        }
    }
}
