#include "EventQueue.hpp"

#ifdef __linux__

template < typename F > void EventQueue::add( int fd, F callback, time_t to ) {
    epoll_event event;
    event.data.fd = fd;
    event.events  = EPOLLIN;
    if ( epoll_ctl( _epoll_fd, EPOLL_CTL_ADD, fd, &event ) == -1 ) {
        throw std::runtime_error( "epoll_ctl" );
    }
    _callbacks[fd] = new Callback< F >( callback, to );
}

template < class F >
EventQueue::Callback< F >::Callback( F f, time_t to )
    : CallbackBase( to ),
      _f( f ) {}

template < class F > void EventQueue::Callback< F >::operator()() { _f(); }

#else

template < typename F >
void EventQueue::add( int fd, F callback, time_t con_to, time_t idle_to ) {
    struct kevent event;
    EV_SET( &event, fd, EVFILT_READ, EV_ADD, 0, 0, 0 );
    if ( kevent( _kqueue_fd, &event, 1, 0, 0, 0 ) == -1 ) {
        throw std::runtime_error( "kevent" );
    }
    _callbacks[fd] = new Callback< F >( callback, con_to, idle_to );
}

template < class F >
EventQueue::Callback< F >::Callback( F f, time_t con_to, time_t idle_to )
    : CallbackBase( con_to, idle_to ),
      _f( f ) {}

template < class F > void EventQueue::Callback< F >::operator()() { _f(); }

#endif
