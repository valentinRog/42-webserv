#pragma once

#include "common.h"

class EventQueue {
    struct HandlerBase {
        virtual void operator()() = 0;
        virtual ~HandlerBase();
    };

    template < typename F > struct Handler : public HandlerBase {
        Handler( F f );
        void operator()();

    private:
        F _f;
    };

public:
    EventQueue( int max_events );
    ~EventQueue();
    template < typename F > void AddEvent( int fd, F callback );
    void                         remove( int fd );
    void                         wait();

private:
    int                            _max_events;
    int                            _epoll_fd;
    std::map< int, HandlerBase * > _callbacks;
    epoll_event *                  _events;
};

#include "../src/EventQueue.tpp"
