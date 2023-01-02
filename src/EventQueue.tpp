#include "EventQueue.hpp"

#ifdef __linux__

template < typename F > void EventQueue::add( int fd, F callback ) {
    epoll_event event;
    event.data.fd = fd;
    event.events  = EPOLLIN;
    if ( epoll_ctl( _epoll_fd, EPOLL_CTL_ADD, fd, &event ) == -1 ) {
        throw std::runtime_error( "epoll_ctl" );
    }
    _callbacks[fd] = new Callback< F >( callback );
}

template < class F > EventQueue::Callback< F >::Callback( F f ) : _f( f ) {}

template < class F > void EventQueue::Callback< F >::operator()() { _f(); }

#else

template < typename F > void EventQueue::add( int fd, F callback, time_t to ) {
    struct kevent event;
    EV_SET( &event, fd, EVFILT_READ, EV_ADD, 0, 0, 0 );
    if ( kevent( _kqueue_fd, &event, 1, 0, 0, 0 ) == -1 ) {
        throw std::runtime_error( "kevent" );
    }
    _callbacks[fd] = new Callback< F >( callback, to );
}

template < class F >
EventQueue::Callback< F >::Callback( F f, time_t to )
    : _f( f ),
      _to( to ),
      _last_t( time( 0 ) ) {}

template < class F > void EventQueue::Callback< F >::operator()() { _f(); }

template < class F > time_t EventQueue::Callback< F >::get_to() const {
    return _to;
}

template < class F > time_t EventQueue::Callback< F >::get_last_t() const {
    return _last_t;
}

template < class F > void EventQueue::Callback< F >::update_last_t() {
    _last_t = time( 0 );
}

#endif
