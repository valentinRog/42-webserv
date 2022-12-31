#include "ServerCluster.hpp"

ServerCluster::ServerCluster() : _q( MAX_EVENTS ) {}

void ServerCluster::bind( uint16_t port ) {
    int        fd = socket( AF_INET, SOCK_STREAM, 0 );
    ServerConf conf( port );
    if ( ::bind( fd,
                 ( sockaddr * ) &conf.get_addr(),
                 sizeof( conf.get_addr() ) )
         == -1 ) {
        throw std::runtime_error( "bind" );
    }
    listen( fd, 100 );
    _q.AddEvent( fd, SocketCallback( fd, conf, _q ) );
}

void ServerCluster::run() {
    while ( ~0 ) { _q.wait(); }
}

ServerCluster::ClientCallback::ClientCallback( int fd, EventQueue &q )
    : _fd( fd ),
      _q( q ) {}

void ServerCluster::ClientCallback::operator()() {
    char   buff[BUFFER_SIZE];
    size_t n = read( _fd, buff, sizeof( buff ) );
    write( STDOUT_FILENO, buff, n );
    fflush( stdout );
    write( _fd, "yo", 2 );
    _q.remove( _fd );
}

ServerCluster::SocketCallback::SocketCallback( int         fd,
                                               ServerConf  conf,
                                               EventQueue &q )
    : _fd( fd ),
      _conf( conf ),
      _q( q ) {}

void ServerCluster::SocketCallback::operator()() {
    socklen_t l  = sizeof( _conf.get_addr() );
    int       fd = accept( _fd, ( sockaddr * ) &_conf.get_addr(), &l );
    _q.AddEvent( fd, ClientCallback( fd, _q ) );
}
