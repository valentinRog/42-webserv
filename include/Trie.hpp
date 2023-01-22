#pragma once

#include "common.h"

/* -------------------------------------------------------------------------- */

class Trie {
    Trie *_children[UCHAR_MAX];
    bool  _eow;

public:
    Trie();
    ~Trie();

    void        insert( const std::string &s );
    bool        search( const std::string &s ) const;
    std::string longest_prefix_subword( const std::string &s ) const;
    std::ostream &repr( std::ostream &os ) const;

private:
    std::ostream &
    _repr_dfs( std::ostream &os, const Trie *t, std::string buff = "" ) const;
};

std::ostream &operator<<( std::ostream &os, const Trie &t );

/* -------------------------------------------------------------------------- */
