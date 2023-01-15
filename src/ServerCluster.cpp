#include "ServerCluster.hpp"

/* -------------------------------------------------------------------------- */

bool ServerCluster::Addr_Less::operator()( const sockaddr_in &lhs,
                                           const sockaddr_in &rhs ) const {
    return ( lhs.sin_addr.s_addr == rhs.sin_addr.s_addr )
               ? lhs.sin_port < rhs.sin_port
               : lhs.sin_addr.s_addr < rhs.sin_addr.s_addr;
}

/* -------------------------------------------------------------------------- */

ServerCluster::ServerCluster() : _q( _max_events ) {}

void ServerCluster::bind( const ServerConf &conf ) {
    std::map< sockaddr_in, VirtualHostMapper, Addr_Less >::iterator it(
        _virtual_hosts.find( conf.get_addr() ) );
    if ( it == _virtual_hosts.end() ) {
        if ( !_ports.count( conf.get_addr().sin_port ) ) {
            _bind( ::ntohs( conf.get_addr().sin_port ) );
        }
        _virtual_hosts.insert(
            std::make_pair( conf.get_addr(), VirtualHostMapper( conf ) ) );
    } else {
        it->second.add( conf );
    }
}

void ServerCluster::run() {
    while ( ~0 ) { _q.wait(); }
}

void ServerCluster::_bind( uint16_t port ) {
    int         fd = ::socket( AF_INET, SOCK_STREAM, 0 );
    sockaddr_in addr;
    ::bzero( &addr, sizeof( addr ) );
    addr.sin_family      = AF_INET;
    addr.sin_port        = ::htons( port );
    addr.sin_addr.s_addr = ::htonl( INADDR_ANY );
    if ( ::bind( fd, reinterpret_cast< sockaddr * >( &addr ), sizeof( addr ) )
         == -1 ) {
        throw std::runtime_error( "bind" );
    }
    ::listen( fd, SOMAXCONN );
    _q.add( fd, SocketCallback( fd, addr, *this ) );
    _ports.insert( addr.sin_port );
}

/* -------------------------------------------------------------------------- */

ServerCluster::ClientCallback::ClientCallback( int                fd,
                                               const sockaddr_in &addr,
                                               ServerCluster &    server,
                                               time_t             con_to,
                                               time_t             idle_to )
    : CallbackBase( con_to, idle_to ),
      _fd( fd ),
      _addr( addr ),
      _server( server ) {
    std::cout << ntohs( _addr.sin_port ) << std::endl;
}

CallbackBase *ServerCluster::ClientCallback::clone() const {
    return new ClientCallback( *this );
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
        _server._q.remove( _fd );
    }
}

void ServerCluster::ClientCallback::handle_timeout() {
    _server._q.remove( _fd );
}

/* -------------------------------------------------------------------------- */

ServerCluster::SocketCallback::SocketCallback( int                fd,
                                               const sockaddr_in &addr,
                                               ServerCluster &    server )
    : CallbackBase( 0, 0 ),
      _fd( fd ),
      _addr( addr ),
      _server( server ) {}

CallbackBase *ServerCluster::SocketCallback::clone() const {
    return new SocketCallback( *this );
}

void ServerCluster::SocketCallback::handle_read() {
    sockaddr_in addr( _addr );
    socklen_t   l = sizeof( addr );
    int fd = ::accept( _fd, reinterpret_cast< sockaddr * >( &addr ), &l );
    getsockname( fd, ( struct sockaddr * ) &addr, &l );
    _server._q.add( fd, ClientCallback( fd, addr, _server ) );
}

void ServerCluster::SocketCallback::handle_write() {}

void ServerCluster::SocketCallback::handle_timeout() {}

/* -------------------------------------------------------------------------- */
