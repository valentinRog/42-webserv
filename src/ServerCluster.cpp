#include "ServerCluster.hpp"

/* -------------------------------------------------------------------------- */

ServerCluster::ServerCluster() : _q( _max_events ) {}

void ServerCluster::bind( uint16_t port ) {
    int        fd = socket( AF_INET, SOCK_STREAM, 0 );
    ServerConf conf( port );
    if ( ::bind( fd, ( sockaddr * ) &conf.get_addr(), sizeof( conf.get_addr() ) ) == -1 ) {
        throw std::runtime_error( "binde" );
    }
    listen( fd, SOMAXCONN );
    _q.add( fd, SocketCallback( fd, conf, _q ) );
}

void ServerCluster::run() {
    while ( ~0 ) { _q.wait(); }
}

/* -------------------------------------------------------------------------- */

ServerCluster::ClientCallback::ClientCallback( int fd, EventQueue &q )
    : CallbackBase( 30, 5 ),
      _fd( fd ),
      _q( q ) {}

CallbackBase *ServerCluster::ClientCallback::clone() const {
    return new ServerCluster::ClientCallback( *this );
}

void ServerCluster::ClientCallback::handle_read() {
    char   buff[_buffer_size];
    size_t n = read( _fd, buff, sizeof( buff ) );
    write( STDOUT_FILENO, buff, n );
    fflush( stdout );
    _s.append( buff, n );
    update_last_t();
}

void ServerCluster::ClientCallback::handle_write() {
    const std::string end = "\r\n\r\n";
    if ( _s.size() >= end.size()
         && !_s.compare( _s.size() - end.size(), end.size(), end ) ) {
        write( _fd, "yo", 2 );
        _q.remove( _fd );
    }
}

void ServerCluster::ClientCallback::handle_timeout() { _q.remove( _fd ); }

/* -------------------------------------------------------------------------- */

ServerCluster::SocketCallback::SocketCallback( int         fd,
                                               ServerConf  conf,
                                               EventQueue &q )
    : CallbackBase( 0, 0 ),
      _fd( fd ),
      _conf( conf ),
      _q( q ) {}

CallbackBase *ServerCluster::SocketCallback::clone() const {
    return new ServerCluster::SocketCallback( *this );
}

void ServerCluster::SocketCallback::handle_read() {
    socklen_t l  = sizeof( _conf.get_addr() );
    int       fd = accept( _fd, ( sockaddr       *) &_conf.get_addr(), &l );
    _q.add( fd, ClientCallback( fd, _q ) );
}

void ServerCluster::SocketCallback::handle_write() {}

void ServerCluster::SocketCallback::handle_timeout() {}

/* -------------------------------------------------------------------------- */
