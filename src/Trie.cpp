#include "Trie.hpp"

/* -------------------------------------------------------------------------- */

Trie::Trie() { ::memset( _children, 0, sizeof( _children ) ); }

Trie::~Trie() {
    for ( size_t i( 0 ); i < UCHAR_MAX; i++ ) {
        if ( _children[i] ) { delete _children[i]; }
    }
}

void Trie::insert( const std::string &s ) {
    Trie *node( this );
    for ( std::string::const_iterator it( s.begin() ); it != s.end(); it++ ) {
        unsigned char i( *it );
        if ( !node->_children[i] ) { node->_children[i] = new Trie(); }
        node = node->_children[i];
    }
    node->_eow = true;
}

bool Trie::search( const std::string &s ) const {
    const Trie *node( this );
    for ( std::string::const_iterator it( s.begin() ); it != s.end(); it++ ) {
        node = node->_children[static_cast< unsigned char >( *it )];
        if ( !node ) { return false; }
    }
    return node->_eow;
}

std::string Trie::longest_prefix_subword( const std::string &s ) const {
    std::string prefix;
    std::string buff;
    const Trie *node( this );
    for ( std::string::const_iterator it( s.begin() ); it != s.end(); it++ ) {
        node = node->_children[static_cast< unsigned char >( *it )];
        if ( !node ) { return prefix; }
        buff += *it;
        if ( node->_eow ) {
            prefix += buff;
            buff.clear();
        }
    }
    return prefix;
}

std::ostream &Trie::repr( std::ostream &os ) const {
    return _repr_dfs( os, this );
}

std::ostream &
Trie::_repr_dfs( std::ostream &os, const Trie *t, std::string buff ) const {
    if ( t->_eow ) { os << buff << std::endl; }
    for ( size_t i = 0; i < UCHAR_MAX; i++ ) {
        if ( t->_children[i] ) {
            _repr_dfs( os, t->_children[i], buff + static_cast< char >( i ) );
        }
    }
    return os;
}

std::ostream &operator<<( std::ostream &os, const Trie &t ) {
    return t.repr(os);
}

/* -------------------------------------------------------------------------- */
