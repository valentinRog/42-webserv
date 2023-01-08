#include "JSON.hpp"

/* -------------------------------------------------------------------------- */

template < typename T, typename I > JSON::Array::Iterator< T, I >::Iterator() {}

template < typename T, typename I >
JSON::Array::Iterator< T, I >::Iterator( const I &it ) : _it( it ) {}

template < typename T, typename I >
JSON::Array::Iterator< T, I >
JSON::Array::Iterator< T, I >::operator+( difference_type n ) const {
    return _it + n;
}

template < typename, typename >
JSON::Array::iterator operator+( JSON::Array::iterator::difference_type lhs,
                                 const JSON::Array::iterator &          rhs ) {
    return rhs + lhs;
}

template < typename, typename >
JSON::Array::const_iterator
operator+( JSON::Array::iterator::difference_type lhs,
           const JSON::Array::const_iterator &    rhs ) {
    return rhs + lhs;
}

template < typename T, typename I >
JSON::Array::Iterator< T, I >
JSON::Array::Iterator< T, I >::operator-( difference_type n ) const {
    return _it - n;
}

template < typename T, typename I >
typename JSON::Array::Iterator< T, I >::difference_type
JSON::Array::Iterator< T, I >::operator-( const Iterator &other ) const {
    return _it - other;
}

template < typename T, typename I >
JSON::Array::Iterator< T, I > &JSON::Array::Iterator< T, I >::operator++() {
    _it++;
    return *this;
}

template < typename T, typename I >
JSON::Array::Iterator< T, I > &JSON::Array::Iterator< T, I >::operator--() {
    _it--;
    return *this;
}

template < typename T, typename I >
JSON::Array::Iterator< T, I > JSON::Array::Iterator< T, I >::operator++( int ) {
    return Iterator( _it++ );
}

template < typename T, typename I >
JSON::Array::Iterator< T, I > JSON::Array::Iterator< T, I >::operator--( int ) {
    return Iterator( _it-- );
}

template < typename T, typename I >
JSON::Array::Iterator< T, I >
JSON::Array::Iterator< T, I >::operator+=( difference_type n ) {
    _it += n;
    return *this;
}

template < typename T, typename I >
JSON::Array::Iterator< T, I >
JSON::Array::Iterator< T, I >::operator-=( difference_type n ) {
    _it -= n;
    return *this;
}

template < typename T, typename I >
bool JSON::Array::Iterator< T, I >::operator==(
    const JSON::Array::const_iterator &rhs ) const {
    return _it == rhs.base();
}

template < typename T, typename I >
bool JSON::Array::Iterator< T, I >::operator!=(
    const JSON::Array::const_iterator &rhs ) const {
    return _it != rhs.base();
}

template < typename T, typename I >
bool JSON::Array::Iterator< T, I >::operator>(
    const JSON::Array::const_iterator &rhs ) const {
    return _it > rhs.base();
}

template < typename T, typename I >
bool JSON::Array::Iterator< T, I >::operator<(
    const JSON::Array::const_iterator &rhs ) const {
    return _it < rhs.base();
}

template < typename T, typename I >
bool JSON::Array::Iterator< T, I >::operator>=(
    const JSON::Array::const_iterator &rhs ) const {
    return _it >= rhs.base();
}

template < typename T, typename I >
bool JSON::Array::Iterator< T, I >::operator<=(
    const JSON::Array::const_iterator &rhs ) const {
    return _it <= rhs.base();
}

template < typename T, typename I >
typename JSON::Array::Iterator< T, I >::reference
JSON::Array::Iterator< T, I >::operator*() {
    return **_it;
}

template < typename T, typename I >
typename JSON::Array::
    Iterator< const T, std::vector< JSON::Value * >::const_iterator >::reference
    JSON::Array::Iterator< T, I >::operator*() const {
    return **_it;
}

template < typename T, typename I >
typename JSON::Array::Iterator< T, I >::reference
JSON::Array::Iterator< T, I >::operator[]( difference_type i ) {
    return _it[i];
}

template < typename T, typename I >
typename JSON::Array::
    Iterator< const T, std::vector< JSON::Value * >::const_iterator >::reference
    JSON::Array::Iterator< T, I >::operator[]( difference_type i ) const {
    return _it[i];
}

template < typename T, typename I >
typename JSON::Array::Iterator< T, I >::pointer
JSON::Array::Iterator< T, I >::operator->() {
    return *_it;
}

template < typename T, typename I >
typename JSON::Array::
    Iterator< const T, std::vector< JSON::Value * >::const_iterator >::pointer
    JSON::Array::Iterator< T, I >::operator->() const {
    return *_it;
}

template < typename T, typename I >
JSON::Array::Iterator< T, I >::operator const_iterator() const {
    return const_iterator( _it );
}

/* -------------------------------------------------------------------------- */

template < typename T > T JSON::Parse::from_string( const std::string &s ) {
    std::queue< std::string > q( _lexer( s ) );
    Value *                   v( _parse( q ) );
    T                         x( *dynamic_cast< T * >( v ) );
    delete v;
    return x;
}

/* -------------------------------------------------------------------------- */
