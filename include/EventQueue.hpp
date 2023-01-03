#pragma once

#include "common.h"

/* -------------------------------------------------------------------------- */

class CallbackBase {
    time_t _con_to;
    time_t _t0;
    time_t _idle_to;
    time_t _last_t;

public:
    CallbackBase( time_t con_to, time_t idle_to );
    virtual void operator()() = 0;
    virtual ~CallbackBase();
    time_t get_con_to() const;
    time_t get_t0() const;
    time_t get_idle_to() const;
    time_t get_last_t() const;
    void   update_last_t();
};

template < typename F > class Callback : public CallbackBase {
    F _f;

public:
    Callback( F f, time_t con_to, time_t idle_to );
    void operator()();
};

template < typename F >
CallbackBase *new_callback( F f, time_t con_to, time_t idle_to ) {
    return new Callback< F >( f, con_to, idle_to );
}

/* -------------------------------------------------------------------------- */

class EventQueueBase {
protected:
    int                             _max_events;
    std::map< int, CallbackBase * > _callbacks;

public:
    virtual void add( int fd, CallbackBase *callback ) = 0;
    virtual void remove( int fd )                      = 0;
    virtual void wait()                                = 0;
    EventQueueBase( int max_events ) : _max_events( max_events ) {}
};

/* -------------------------------------------------------------------------- */

#ifdef __linux__

/* -------------------------------------------------------------------------- */

class EventQueue : EventEventQueueBase {
public:
    EventQueue( int max_events );
    ~EventQueue();
    template < typename F > void add( int fd, CallbackBase *callback );
    void                         remove( int fd );
    void                         wait();

private:
    int          _epoll_fd;
    epoll_event *_events;
};

/* -------------------------------------------------------------------------- */

#else

/* -------------------------------------------------------------------------- */

class EventQueue : EventQueueBase {
public:
    EventQueue( int max_events );
    ~EventQueue();
    void add( int fd, CallbackBase *callback );
    void remove( int fd );
    void wait();

private:
    int            _kqueue_fd;
    struct kevent *_events;
};

/* -------------------------------------------------------------------------- */

#endif

#include "../src/EventQueue.tpp"
