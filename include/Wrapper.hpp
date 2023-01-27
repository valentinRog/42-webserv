#pragma once

/* -------------------------------------------------------------------------- */

template < typename T > class PolymorphicWrapper {
    T *_p;

public:
    PolymorphicWrapper( const T &v );
    PolymorphicWrapper( const PolymorphicWrapper &other );
    ~PolymorphicWrapper();
    PolymorphicWrapper &operator=( const PolymorphicWrapper &other );

    T &      operator*();
    const T &operator*() const;

    T *      operator->();
    const T *operator->() const;

    template < typename U > U &      unwrap();
    template < typename U > const U &unwrap() const;
};

/* -------------------------------------------------------------------------- */

class RunOnce {
private:
    bool _ran;

public:
    RunOnce() : _ran( false ) {}

    template < typename F > void run( F &fn ) {
        if ( !_ran ) {
            fn();
            _ran = true;
        }
    }
};

/* -------------------------------------------------------------------------- */

#include "Wrapper.tpp"
