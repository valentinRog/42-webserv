#pragma once

#include "PolymorphicWrapper.hpp"
#include "Trait.hpp"
#include "common.h"

/* ------------------------------ CallbackBase ------------------------------ */

class CallbackBase : public Trait::CloneCRTP< CallbackBase > {
    time_t _con_to;
    time_t _t0;
    time_t _idle_to;
    time_t _last_t;

public:
    CallbackBase( time_t con_to, time_t idle_to );
    virtual ~CallbackBase();

    virtual void handle_read()    = 0;
    virtual void handle_write()   = 0;
    virtual void handle_timeout() = 0;

    virtual CallbackBase *clone() const = 0;

    time_t con_to() const;
    time_t t0() const;
    time_t idle_to() const;
    time_t last_t() const;

protected:
    void update_last_t();
};

/* ----------------------------- EventQueueBase ----------------------------- */

class EventQueueBase {
protected:
    int                                                 _max_events;
    std::map< int, PolymorphicWrapper< CallbackBase > > _callbacks;

public:
    virtual ~EventQueueBase();
    virtual void add( int fd, const CallbackBase &callback ) = 0;
    virtual void remove( int fd )                            = 0;
    virtual void wait()                                      = 0;
    EventQueueBase( int max_events ) : _max_events( max_events ) {}
};

/* -------------------------------------------------------------------------- */

#ifdef __linux__

/* ------------------------------- EventQueue ------------------------------- */

class EventQueue : EventQueueBase {
public:
    EventQueue( int max_events );
    ~EventQueue();
    void add( int fd, const CallbackBase &callback );
    void remove( int fd );
    void wait();

private:
    int          _epoll_fd;
    epoll_event *_events;
};

/* -------------------------------------------------------------------------- */

#else

/* ------------------------------- EventQueue ------------------------------- */

class EventQueue : EventQueueBase {
public:
    EventQueue( int max_events );
    ~EventQueue();
    void add( int fd, const CallbackBase &callback );
    void remove( int fd );
    void wait();

private:
    int            _kqueue_fd;
    struct kevent *_events;
};

/* -------------------------------------------------------------------------- */

#endif
