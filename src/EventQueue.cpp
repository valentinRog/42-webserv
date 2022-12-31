#include "EventQueue.hpp"

#ifdef __linux__

EventQueue::EventQueue( int max_events ) : _max_events( max_events ) {
    _epoll_fd = epoll_create( _max_events );
    if ( _epoll_fd == -1 ) { throw std::runtime_error( "epoll_create" ); }
    _events = new epoll_event[_max_events];
}

EventQueue::~EventQueue() {
    close( _epoll_fd );
    delete[] _events;
}

void EventQueue::remove( int fd ) {
    if ( epoll_ctl( _epoll_fd, EPOLL_CTL_DEL, fd, 0 ) == -1 ) {
        throw std::runtime_error( "epoll_ctl" );
    }
    close( fd );
    delete _callbacks[fd];
    _callbacks.erase( fd );
}

void EventQueue::wait() {
    int n_events = epoll_wait( _epoll_fd, _events, _max_events, -1 );
    if ( n_events == -1 ) { throw std::runtime_error( "epoll_wait" ); }
    for ( int i = 0; i < n_events; i++ ) {
        ( *_callbacks[_events[i].data.fd] )();
    }
}

EventQueue::HandlerBase::~HandlerBase() {}

#else

EventQueue::EventQueue( int max_events ) : _max_events( max_events ) {
    _kqueue_fd = kqueue();
    if ( _kqueue_fd == -1 ) { throw std::runtime_error( "kqueue" ); }

    _events = new struct kevent[_max_events];
}

EventQueue::~EventQueue() {
    close( _kqueue_fd );
    delete[] _events;
}

void EventQueue::remove( int fd ) {
    struct kevent event;
    EV_SET( &event, fd, EVFILT_READ, EV_DELETE, 0, 0, 0 );
    if ( kevent( _kqueue_fd, &event, 1, 0, 0, 0 ) == -1 ) {
        throw std::runtime_error( "kevent" );
    }
    close( fd );
    delete _callbacks[fd];
    _callbacks.erase( fd );
}

void EventQueue::wait() {
    int n_events = kevent( _kqueue_fd, 0, 0, _events, _max_events, 0 );
    if ( n_events == -1 ) { throw std::runtime_error( "kevent" ); }
    for ( int i = 0; i < n_events; i++ ) {
        ( *_callbacks[_events[i].ident] )();
    }
}

EventQueue::HandlerBase::~HandlerBase() {}

#endif
