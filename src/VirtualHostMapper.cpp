#include "VirtualHostMapper.hpp"

/* -------------------------------------------------------------------------- */

VirtualHostMapper::VirtualHostMapper( const ServerConf &default_conf ) {
    add( default_conf );
}

VirtualHostMapper::VirtualHostMapper( const VirtualHostMapper &other ) {
    *this = other;
}

VirtualHostMapper &
VirtualHostMapper::operator=( const VirtualHostMapper &other ) {
    _conf.clear();
    _names_map.clear();
    for ( std::vector< ServerConf >::const_iterator it( other._conf.begin() );
          it != other._conf.end();
          it++ ) {
        add( *it );
    }
    return *this;
}

const ServerConf &VirtualHostMapper::get_default() const {
    return _conf.front();
}

const ServerConf &VirtualHostMapper::operator[]( const std::string &s ) const {
    std::map< std::string, ServerConf * >::const_iterator it(
        _names_map.find( s ) );
    return it == _names_map.end() ? get_default() : *it->second;
}

void VirtualHostMapper::add( const ServerConf &conf ) {
    _conf.push_back( conf );
    for ( std::set< std::string >::const_iterator it(
              conf.get_names().begin() );
          it != conf.get_names().end();
          it++ ) {
        _names_map.insert( std::make_pair( *it, &_conf.back() ) );
    }
}

/* -------------------------------------------------------------------------- */
