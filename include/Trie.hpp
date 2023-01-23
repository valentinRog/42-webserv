#pragma once

#include "common.h"

/* -------------------------------------------------------------------------- */

class Trie {
    Trie *_children[UCHAR_MAX];
    bool  _eow;

public:
    Trie();
    Trie( const Trie &other );
    Trie &operator=( const Trie &other );
    ~Trie();

    void          insert( const std::string &s );
    bool          search( const std::string &s ) const;
    std::string   lower_bound( const std::string &s ) const;
    std::ostream &repr( std::ostream &os ) const;

private:
    std::ostream &_repr_dfs( std::ostream &os,
                             const Trie *  t,
                             bool &        first,
                             std::string   buff = "" ) const;
};

std::ostream &operator<<( std::ostream &os, const Trie &t );

/* -------------------------------------------------------------------------- */
