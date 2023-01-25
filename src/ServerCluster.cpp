#include "ServerCluster.hpp"

/* -------------------------------------------------------------------------- */

ServerCluster::ServerCluster() : _q( _max_events ) {}

void ServerCluster::bind( const ServerConf &conf ) {
    uint16_t port( conf.addr.sin_port );
    uint32_t addr( conf.addr.sin_addr.s_addr );
    if ( !_vh.count( port ) ) {
        _bind( ntohs( port ) );
        _vh[port];
    }
    if ( _vh.at( port ).count( addr ) ) {
        _vh.at( port ).at( addr ).add( conf );
    } else {
        _vh.at( port ).insert(
            std::make_pair( addr, VirtualHostMapper( conf ) ) );
    }
}

void ServerCluster::run() {
    while ( ~0 ) { _q.wait(); }
}

void ServerCluster::_bind( uint16_t port ) {
    int fd = ::socket( AF_INET, SOCK_STREAM, 0 );
    if ( fd < 0 ) { throw std::runtime_error( "socket" ); }
    sockaddr_in addr;
    ::bzero( &addr, sizeof( addr ) );
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons( port );
    addr.sin_addr.s_addr = htonl( INADDR_ANY );
    int ra               = 1;
    if ( ::setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &ra, sizeof( ra ) ) < 0 ) {
        throw std::runtime_error( "setsockopt" );
    }
    std::cout << "binding " << port << std::endl;
    if ( ::bind( fd, reinterpret_cast< sockaddr * >( &addr ), sizeof( addr ) )
         == -1 ) {
        throw std::runtime_error( "bind" );
    }
    if ( ::listen( fd, SOMAXCONN ) < 0 ) {
        throw std::runtime_error( "listen" );
    }
    _q.add( fd, SocketCallback( fd, addr, *this ) );
}

/* -------------------------------------------------------------------------- */

ServerCluster::ClientCallback::ClientCallback( int                      fd,
                                               const sockaddr_in &      addr,
                                               ServerCluster &          server,
                                               const VirtualHostMapper &vhm,
                                               time_t                   con_to,
                                               time_t idle_to )
    : CallbackBase( con_to, idle_to ),
      _fd( fd ),
      _addr( addr ),
      _server( server ),
      _vhm( vhm ) {}

CallbackBase *ServerCluster::ClientCallback::clone() const {
    return new ClientCallback( *this );
}

void ServerCluster::ClientCallback::handle_read() {
    char        buff[_buffer_size];
    size_t      n = read( _fd, buff, sizeof( buff ) );
    std::string s;
    s.append( buff, n );
    _http_parser << s;
    update_last_t();
}

void ServerCluster::ClientCallback::handle_write() {
    if ( _http_parser.step() == HTTP::DynamicParser::DONE ) {
        HTTP::RequestHandler rh( _http_parser.request(), _vhm );
        std::string          response = rh.getResponse().stringify();
        write( _fd, response.c_str(), response.size() );
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
    getsockname( fd, reinterpret_cast< sockaddr * >( &addr ), &l );
    typedef std::map< u_int32_t, VirtualHostMapper > map_type;
    const map_type &         m( _server._vh.at( addr.sin_port ) );
    map_type::const_iterator it = m.find( addr.sin_addr.s_addr );
    if ( it == m.end() ) { it = m.find( htonl( INADDR_ANY ) ); }
    if ( it == m.end() ) {
        close( fd );
    } else {
        _server._q.add( fd, ClientCallback( fd, addr, _server, it->second ) );
    }
}

void ServerCluster::SocketCallback::handle_write() {}

void ServerCluster::SocketCallback::handle_timeout() {}

/* -------------------------------------------------------------------------- */
