#include "ServerCluster.hpp"

/* -------------------- ServerCluster::VirtualHostMapper -------------------- */

ServerCluster::VirtualHostMapper::VirtualHostMapper(
    const ServerConf &default_conf )
    : _default( new ServerConf( default_conf ) ) {}

Ptr::Shared< ServerConf >
ServerCluster::VirtualHostMapper::get_default() const {
    return _default;
}

Ptr::Shared< ServerConf >
ServerCluster::VirtualHostMapper::operator[]( const std::string &s ) const {
    std::map< std::string, Ptr::Shared< ServerConf > >::const_iterator it(
        _names_map.find( s ) );
    return it == _names_map.end() ? _default : it->second;
}

void ServerCluster::VirtualHostMapper::add( const ServerConf &conf ) {
    for ( std::set< std::string >::const_iterator it( conf.names().begin() );
          it != conf.names().end();
          it++ ) {
        _names_map[*it] = new ServerConf( conf );
    }
}

/* ------------------------------ ServerCluster ----------------------------- */

ServerCluster::ServerCluster() : _q( _max_events ) {}

void ServerCluster::bind( const ServerConf &conf ) {
    uint16_t port( conf.addr().sin_port );
    uint32_t addr( conf.addr().sin_addr.s_addr );
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
    if ( ::bind( fd, reinterpret_cast< sockaddr * >( &addr ), sizeof( addr ) )
         == -1 ) {
        throw std::runtime_error( "bind" );
    }
    if ( ::listen( fd, SOMAXCONN ) < 0 ) {
        throw std::runtime_error( "listen" );
    }
    _q.add( fd, SocketCallback( fd, addr, *this ) );
}

/* ---------------------- ServerCluster::ClientCallback --------------------- */

ServerCluster::ClientCallback::ClientCallback( int                      fd,
                                               const VirtualHostMapper &vhm,
                                               time_t                   con_to,
                                               time_t idle_to )
    : CallbackBase( con_to, idle_to ),
      _fd( fd ),
      _vhm( vhm ) {}

CallbackBase *ServerCluster::ClientCallback::clone() const {
    return new ClientCallback( *this );
}

void ServerCluster::ClientCallback::handle_read() {
    char   buff[_buffer_size];
    size_t n( read( _fd, buff, sizeof( buff ) ) );
    _http_parser.add( buff, n );
    update_last_t();
}

void ServerCluster::ClientCallback::handle_write() {
    if ( !_http_parser.done() && !_http_parser.failed() ) { return; }
    std::cout << CYAN << '[' << _fd << ']' << RESET << ' ';
    if ( _http_parser.failed() ) {
        std::string response( HTTP::Response::make_error_response(
                                  _http_parser.error(),
                                  _vhm.get_default()->code_to_error_page() )
                                  .stringify() );
        write( _fd, response.c_str(), response.size() );
        std::cout << RED
                  << HTTP::Response::code_to_string().at( _http_parser.error() )
                  << RESET << ' '
                  << HTTP::Response::code_to_message( _http_parser.error() )
                  << std::endl;
        if ( !_http_parser.request()->keep_alive() ) {
            kill_me();
            std::cout << CYAN << '[' << _fd << ']' << RESET << " closed\n";
        } else {
            _http_parser = HTTP::Request::DynamicParser();
        }
    } else {
        std::cout << HTTP::Request::method_to_string().at(
            _http_parser.request()->method() )
                  << ' ' << _http_parser.request()->url() << BLUE << " -> "
                  << RESET;
        Ptr::Shared< HTTP::Request > request( _http_parser.request() );
        RequestHandler               rh( request, _vhm[request->host()] );
        HTTP::Response               response( rh.make_response() );
        std::string                  s( response.stringify() );
        write( _fd, s.c_str(), s.size() );
        std::cout << ( response.code == HTTP::Response::E200 ? GREEN : RED );
        std::cout << HTTP::Response::code_to_string().at( response.code )
                  << RESET << ' '
                  << HTTP::Response::code_to_message( response.code )
                  << std::endl;
        if ( !_http_parser.request()->keep_alive() ) {
            kill_me();
            std::cout << CYAN << '[' << _fd << ']' << YELLOW << " closed"
                      << RESET << std::endl;
        } else {
            _http_parser = HTTP::Request::DynamicParser();
        }
    }
}

void ServerCluster::ClientCallback::handle_timeout() {
    kill_me();
    std::cout << CYAN << '[' << _fd << ']' << YELLOW << " timed out" << RESET
              << std::endl;
}

/* ---------------------- ServerCluster::SocketCallback --------------------- */

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
    if ( fd < 0 || fcntl( fd, F_SETFL, O_NONBLOCK ) < 0 ) { return; }
    char buff[INET_ADDRSTRLEN];
    inet_ntop( AF_INET, &addr.sin_addr, buff, sizeof( buff ) );
    std::cout << CYAN << '[' << fd << "] " << RESET << buff << ':'
              << ntohs( _addr.sin_port ) << BLUE << " -> " << YELLOW
              << " created tcp connection\n"
              << RESET;
    getsockname( fd, reinterpret_cast< sockaddr * >( &addr ), &l );
    typedef std::map< u_int32_t, VirtualHostMapper > map_type;
    const map_type &         m( _server._vh.at( addr.sin_port ) );
    map_type::const_iterator it = m.find( addr.sin_addr.s_addr );
    if ( it == m.end() ) { it = m.find( htonl( INADDR_ANY ) ); }
    if ( it == m.end() ) {
        close( fd );
    } else {
        _server._q.add( fd, ClientCallback( fd, it->second ) );
    }
}

void ServerCluster::SocketCallback::handle_write() {}

void ServerCluster::SocketCallback::handle_timeout() {}

/* -------------------------------------------------------------------------- */
