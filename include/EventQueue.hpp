#pragma once

#include "common.h"

#ifdef __linux__

class EventQueue {
    struct CallbackBase {
        CallbackBase( time_t con_to, time_t idle_to );
        virtual void operator()() = 0;
        virtual ~CallbackBase();
        time_t get_con_to() const;
        time_t get_t0() const;
        time_t get_idle_to() const;
        time_t get_last_t() const;
        void   update_last_t();

    private:
        time_t _con_to;
        time_t _t0;
        time_t _idle_to;
        time_t _last_t;
    };

    template < typename F > struct Callback : public CallbackBase {
        Callback( F f, time_t con_to, time_t idle_to );
        void operator()();

    private:
        F _f;
    };

public:
    EventQueue( int max_events );
    ~EventQueue();
    template < typename F >
    void add( int fd, F callback, time_t con_to = 0, time_t idle_to = 0 );
    void remove( int fd );
    void wait();

private:
    int                             _max_events;
    int                             _epoll_fd;
    std::map< int, CallbackBase * > _callbacks;
    epoll_event *                   _events;
};

#else

class EventQueue {
    struct CallbackBase {
        CallbackBase( time_t con_to, time_t idle_to );
        virtual void operator()() = 0;
        virtual ~CallbackBase();
        time_t get_con_to() const;
        time_t get_t0() const;
        time_t get_idle_to() const;
        time_t get_last_t() const;
        void   update_last_t();

    private:
        time_t _con_to;
        time_t _t0;
        time_t _idle_to;
        time_t _last_t;
    };

    template < typename F > struct Callback : public CallbackBase {
        Callback( F f, time_t con_to, time_t idle_to );
        void operator()();

    private:
        F _f;
    };

public:
    EventQueue( int max_events );
    ~EventQueue();
    template < typename F >
    void add( int fd, F callback, time_t con_to = 0, time_t idle_to = 0 );
    void remove( int fd );
    void wait();

private:
    int                             _max_events;
    int                             _kqueue_fd;
    std::map< int, CallbackBase * > _callbacks;
    struct kevent *                 _events;
};

#endif

#include "../src/EventQueue.tpp"
