#include "ServerCluster.hpp"

/* ------------------------------ ServerCluster ----------------------------- */

const std::string &
ServerCluster::key_to_string( ServerCluster::e_config_key key ) {
    typedef std::map< e_config_key, std::string > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[MIME_FILE] = "mime_file";
            m[SERVERS]   = "servers";
            return m;
        }
    };
    static const map_type m( f::init() );
    return m.at( key );
}

const std::map< std::string, ServerCluster::e_config_key > &
ServerCluster::string_to_key() {
    typedef std::map< std::string, e_config_key > map_type;
    struct f {
        static map_type init() {
            map_type m;
            m[key_to_string( MIME_FILE )] = MIME_FILE;
            m[key_to_string( SERVERS )]   = SERVERS;
            return m;
        }
    };
    static const map_type m( f::init() );
    return m;
}

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

ServerCluster::ServerCluster( const JSON::Object &o ) : _q( _max_events ) {
    for ( JSON::Object::const_iterator it( o.begin() ); it != o.end(); it++ ) {
        string_to_key().at( it->first );
    }
    Ptr::Shared< std::map< std::string, std::string > > mime(
        new std::map< std::string, std::string > );
    JSON::Object mime_o(
        JSON::Parse::from_file(
            o.at( key_to_string( MIME_FILE ) ).unwrap< JSON::String >() )
            .unwrap< JSON::Object >() );
    for ( JSON::Object::const_iterator it( mime_o.begin() ); it != mime_o.end();
          it++ ) {
        JSON::Array a( it->second.unwrap< JSON::Array >() );
        for ( JSON::Array::const_iterator nit( a.begin() ); nit != a.end();
              nit++ ) {
            ( *mime )[nit->unwrap< JSON::String >()] = it->first;
        }
    }
    JSON::Array a( o.at( key_to_string( SERVERS ) ).unwrap< JSON::Array >() );
    std::vector< ServerConf > v;
    for ( JSON::Array::const_iterator it( a.begin() ); it != a.end(); it++ ) {
        v.push_back( ServerConf( it->unwrap< JSON::Object >(), mime ) );
    }
    for ( std::vector< ServerConf >::const_iterator it = v.begin();
          it != v.end();
          it++ ) {
        bind( *it );
    }
}

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

/* ---------------------- ServerCluster::ClientCallback --------------------- */

ServerCluster::ClientCallback::ClientCallback( int                      fd,
                                               ServerCluster           &server,
                                               const VirtualHostMapper &vhm,
                                               time_t                   con_to,
                                               time_t idle_to )
    : CallbackBase( con_to, idle_to ),
      _fd( fd ),
      _server( server ),
      _vhm( vhm ) {}

CallbackBase *ServerCluster::ClientCallback::clone() const {
    return new ClientCallback( *this );
}

void ServerCluster::ClientCallback::handle_read() {
    char   buff[_buffer_size];
    size_t n( read( _fd, buff, sizeof( buff ) ) );
    write( STDOUT_FILENO, buff, n );
    _http_parser.add( buff, n );
    update_last_t();
}

void ServerCluster::ClientCallback::handle_write() {
    if ( _http_parser.step() == HTTP::Request::DynamicParser::FAILED ) {
        std::string response( RequestHandler::make_error_response(
                                  _http_parser.error(),
                                  _vhm.get_default().operator->() )
                                  .stringify() );
        std::cout << write( _fd, response.c_str(), response.size() )
                  << std::endl;
        std::cout << response << std::endl;
        _server._q.remove( _fd );
    } else if ( _http_parser.step() == HTTP::Request::DynamicParser::DONE ) {
        Ptr::Shared< HTTP::Request > request( _http_parser.request() );
        std::cout << request->content() << std::endl;
        RequestHandler rh( request, _vhm[request->host()] );
        std::string    response = rh.make_raw_response();
        write( _fd, response.c_str(), response.size() );
        _server._q.remove( _fd );
    }
}

void ServerCluster::ClientCallback::handle_timeout() {
    _server._q.remove( _fd );
}

/* ---------------------- ServerCluster::SocketCallback --------------------- */

ServerCluster::SocketCallback::SocketCallback( int                fd,
                                               const sockaddr_in &addr,
                                               ServerCluster     &server )
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
    const map_type          &m( _server._vh.at( addr.sin_port ) );
    map_type::const_iterator it = m.find( addr.sin_addr.s_addr );
    if ( it == m.end() ) { it = m.find( htonl( INADDR_ANY ) ); }
    if ( it == m.end() ) {
        close( fd );
    } else {
        _server._q.add( fd, ClientCallback( fd, _server, it->second ) );
    }
}

void ServerCluster::SocketCallback::handle_write() {}

void ServerCluster::SocketCallback::handle_timeout() {}

/* -------------------------------------------------------------------------- */
