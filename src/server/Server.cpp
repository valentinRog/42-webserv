#include "Server.hpp"

Server::Server() { _kq = kqueue(); }

void Server::bind( uint16_t port ) {
    int fd = socket( AF_INET, SOCK_STREAM, 0 );
    _servers.insert( std::make_pair( fd, ServerConf( port ) ) );
    if ( ::bind( fd,
                 ( sockaddr * ) &_servers.find( fd )->second.get_addr(),
                 sizeof( sockaddr_in ) )
         < 0 ) {
        perror( "bind" );
        exit( EXIT_FAILURE );
    }
    listen( fd, 100 );
    EV_SET( _change_event, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0 );
    kevent( _kq, _change_event, 1, NULL, 0, NULL );
}

void Server::run() {
    while ( ~0 ) {
        int ne = kevent( _kq, NULL, 0, _event, 1, NULL );
        for ( int i = 0; i < ne; i++ ) {
            int efd = _event[i].ident;
            if ( _servers.find( efd ) != _servers.end() ) {
                socklen_t l   = sizeof( sockaddr_in );
                int       cfd = accept(
                    efd,
                    ( sockaddr       *) &_servers.find( efd )->second.get_addr(),
                    &l );
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
