#include "ServerCluster.hpp"

/* -------------------- ServerCluster::VirtualHostMapper -------------------- */

ServerCluster::VirtualHostMapper::VirtualHostMapper(
    const ServerConf &default_conf )
    : _default( new ServerConf( default_conf ) ) {
    add( default_conf );
}

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

size_t ServerCluster::VirtualHostMapper::add( const ServerConf &conf ) {
    size_t n = 0;
    for ( std::set< std::string >::const_iterator it( conf.names().begin() );
          it != conf.names().end();
          it++ ) {
        if ( _names_map.count( *it ) ) { continue; }
        _names_map[*it] = new ServerConf( conf );
        n++;
    }
    return n;
}

/* ------------------------------ ServerCluster ----------------------------- */

ServerCluster::ServerCluster() : _q( _MAX_EVENTS ) {}

bool ServerCluster::bind( const ServerConf &conf ) {
    uint16_t port( conf.addr().sin_port );
    uint32_t addr( conf.addr().sin_addr.s_addr );
    if ( !_vh.count( port ) ) {
        if ( _bind( ntohs( port ) ) ) { return true; }
        _vh[port];
    }
    if ( _vh.at( port ).count( addr ) ) {
        size_t ret = _vh.at( port ).at( addr ).add( conf );
        return ret ? false : true;
        return _vh.at( port ).at( addr ).add( conf ) ? false : true;
    }
    _vh.at( port ).insert( std::make_pair( addr, VirtualHostMapper( conf ) ) );
    return false;
}

void ServerCluster::run() {
    if ( !_vh.empty() ) {
        while ( ~0 ) { _q.wait(); }
    }
}

bool ServerCluster::_bind( uint16_t port ) {
    int fd = ::socket( AF_INET, SOCK_STREAM, 0 );
    if ( fd < 0 ) { return true; }
    sockaddr_in addr;
    ::bzero( &addr, sizeof( addr ) );
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons( port );
    addr.sin_addr.s_addr = htonl( INADDR_ANY );
    int ra               = 1;
    if ( ::setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &ra, sizeof( ra ) ) < 0 ) {
        return true;
    }
    if ( ::bind( fd, reinterpret_cast< sockaddr * >( &addr ), sizeof( addr ) )
         == -1 ) {
        return true;
    }
    if ( ::listen( fd, SOMAXCONN ) < 0 ) { return true; }
    _q.add( fd, SocketCallback( fd, addr, *this ) );
    return false;
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
    const BiMap< HTTP::Header::e_key, std::string > &m
        = HTTP::Header::key_to_string();
    const std::string CONTENT_LENGTH = m.at( HTTP::Header::CONTENT_LENGTH );
    const std::string HOST           = m.at( HTTP::Header::HOST );
    const std::string TRANSFER_ENCODING
        = m.at( HTTP::Header::TRANSFER_ENCODING );

    update_last_t();
    char    buff[_BUFFER_SIZE];
    ssize_t n( read( _fd, buff, sizeof( buff ) ) );
    if ( _check_io( n ) ) { return; }
    size_t ret( 0 );
    if ( _request.is_none() ) {
        ret = Str::append_until( _raw_request, buff, buff + n, "\r\n" );
        ret += Str::append_until( _raw_header,
                                  buff + ret,
                                  buff + n,
                                  "\r\n\r\n" );
        if ( Str::ends_with( _raw_header, "\r\n\r\n" ) ) {
            _request = HTTP::Request::from_string( _raw_request, _raw_header );
            if ( _request.is_none() ) {
                _error = HTTP::Response::E400;
                return;
            }
        }
    }
    if ( _request.is_some() && _accu.is_none()
         && _request.unwrap().header().count( CONTENT_LENGTH ) ) {
        size_t l;
        std::istringstream( _request.unwrap().header().at( CONTENT_LENGTH ) )
            >> l;
        size_t max_body_size = _vhm[_request.unwrap().header().get( HOST )]
                                   ->client_max_body_size();
        _accu = PolymorphicWrapper< HTTP::ContentAccumulatorBase >(
            HTTP::ContentAccumulator( l, max_body_size ) );
    } else if ( _request.is_some() && _accu.is_none()
                && _request.unwrap().header().count( TRANSFER_ENCODING ) ) {
        size_t max_body_size = _vhm[_request.unwrap().header().get( HOST )]
                                   ->client_max_body_size();
        _accu = PolymorphicWrapper< HTTP::ContentAccumulatorBase >(
            HTTP::ChunkedContentAccumulator( max_body_size ) );
    }
    if ( _accu.is_some() ) {
        _accu.unwrap()->feed( buff + ret, buff + n );
        if ( _accu.unwrap()->done() ) {
            _raw_content = _accu.unwrap()->content();
            if ( _accu.unwrap()->failed() ) { _error = HTTP::Response::E413; }
            _accu = Option<
                PolymorphicWrapper< HTTP::ContentAccumulatorBase > >();
        }
    }
}

void ServerCluster::ClientCallback::handle_write() {
    const BiMap< HTTP::Header::e_key, std::string > &m
        = HTTP::Header::key_to_string();
    const std::string HOST = m.at( HTTP::Header::HOST );

    if ( _error.is_some() ) {
        std::string s = HTTP::Response::make_error_response( _error.unwrap() )
                            .stringify();
        if ( _check_io( write( _fd, s.c_str(), s.size() ) ) ) {
            return;
        }
        kill_me();
        std::cout << CYAN << '[' << _fd << ']' << YELLOW << " closed" << RESET
                  << std::endl;
        _log_write_failure( _error.unwrap() );
        return;
    }
    if ( _request.is_none() || _accu.is_some() ) { return; }
    Ptr::Shared< HTTP::Request > r = new HTTP::Request(
        HTTP::Request::from_string( _raw_request, _raw_header ).unwrap() );
    r->set_content( _raw_content );
    RequestHandler rh( r, _vhm[_request.unwrap().header().get( HOST )] );
    HTTP::Response response( rh.make_response() );
    std::string    s( response.stringify() );
    if ( _check_io( write( _fd, s.c_str(), s.size() ) ) ) { return; }
    _log_write_response( response.code );
    if ( _request.unwrap().header().get(
             HTTP::Header::key_to_string().at( HTTP::Header::CONNECTION ) )
         == "keep-alive" ) {
        _request = Option< HTTP::Request >();
        _raw_request.clear();
        _raw_header.clear();
        _raw_content = 0;
        return;
    }
    kill_me();
    std::cout << CYAN << '[' << _fd << ']' << YELLOW << " closed" << RESET
              << std::endl;
}

void ServerCluster::ClientCallback::handle_timeout() {
    kill_me();
    std::cout << CYAN << '[' << _fd << ']' << YELLOW << " timed out" << RESET
              << std::endl;
}

void ServerCluster::ClientCallback::_log_write_failure(
    HTTP::Response::e_error_code code ) const {
    std::cout << CYAN << '[' << _fd << ']' << RESET << ' ' << RED
              << HTTP::Response::code_to_string().at( code ) << RESET << ' '
              << HTTP::Response::code_to_message( code ) << std::endl;
}

void ServerCluster::ClientCallback::_log_write_response(
    HTTP::Response::e_error_code code ) const {
    std::cout << CYAN << '[' << _fd << ']' << RESET << ' '
              << _request.unwrap().method() << ' ' << _request.unwrap().url()
              << BLUE << " -> " << RESET
              << ( code == HTTP::Response::E200 ? GREEN : RED )
              << HTTP::Response::code_to_string().at( code ) << RESET << ' '
              << HTTP::Response::code_to_message( code ) << std::endl;
}

bool ServerCluster::ClientCallback::_check_io( ssize_t            ret) {
    if ( ret > 0 ) { return false; }
    kill_me();
    std::cout << CYAN << '[' << _fd << ']' << YELLOW << " closed" << RESET
              << std::endl;
    return true;
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
              << " created tcp connection" << RESET << std::endl;
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
