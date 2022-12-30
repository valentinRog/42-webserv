#include "EventQueue.hpp"

template < typename F > void EventQueue::AddEvent( int fd, F callback ) {
    epoll_event event;
    event.data.fd = fd;
    event.events  = EPOLLIN;
    if ( epoll_ctl( _epoll_fd, EPOLL_CTL_ADD, fd, &event ) == -1 ) {
        throw std::runtime_error( "epoll_ctl" );
    }
    _callbacks[fd] = new Handler< F >( callback );
}

template < class F > EventQueue::Handler< F >::Handler( F f ) : _f( f ) {}

template < class F > void EventQueue::Handler< F >::operator()() { _f(); }
