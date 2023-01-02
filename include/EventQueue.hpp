#pragma once

#include "common.h"

#ifdef __linux__

class EventQueue {
    struct CallbackBase {
        virtual void operator()() = 0;
        virtual ~CallbackBase();
        virtual time_t get_to() const     = 0;
        virtual time_t get_last_t() const = 0;
        virtual void   update_last_t()    = 0;
    };

    template < typename F > struct Callback : public CallbackBase {
        Callback( F f, time_t to );
        void operator()();
        time_t get_to() const;
        time_t get_last_t() const;
        void   update_last_t();

    private:
        F _f;
        time_t _to;
        time_t _last_t;
    };

public:
    EventQueue( int max_events );
    ~EventQueue();
    template < typename F > void add( int fd, F callback, time_t to = 0 );
    void                         remove( int fd );
    void                         wait();

private:
    int                             _max_events;
    int                             _epoll_fd;
    std::map< int, CallbackBase * > _callbacks;
    epoll_event                    *_events;
};

#else

class EventQueue {
    struct CallbackBase {
        virtual void operator()() = 0;
        virtual ~CallbackBase();
        virtual time_t get_to() const     = 0;
        virtual time_t get_last_t() const = 0;
        virtual void   update_last_t()    = 0;
    };

    template < typename F > struct Callback : public CallbackBase {
        Callback( F f, time_t to );
        void   operator()();
        time_t get_to() const;
        time_t get_last_t() const;
        void   update_last_t();

    private:
        F      _f;
        time_t _to;
        time_t _last_t;
    };

public:
    EventQueue( int max_events );
    ~EventQueue();
    template < typename F > void add( int fd, F callback, time_t to = 0 );
    void                         remove( int fd );
    void                         wait();

private:
    int                             _max_events;
    int                             _kqueue_fd;
    std::map< int, CallbackBase * > _callbacks;
    struct kevent                  *_events;
};

#endif

#include "../src/EventQueue.tpp"
