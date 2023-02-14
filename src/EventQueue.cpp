#include "EventQueue.hpp"

/* ------------------------------ CallbackBase ------------------------------ */

CallbackBase::CallbackBase( time_t con_to, time_t idle_to )
    : _con_to( con_to ),
      _t0( time( 0 ) ),
      _idle_to( idle_to ),
      _last_t( time( 0 ) ),
      _kill_me( false ) {}

CallbackBase::~CallbackBase() {}

time_t CallbackBase::con_to() const { return _con_to; }
time_t CallbackBase::t0() const { return _t0; }
time_t CallbackBase::idle_to() const { return _idle_to; }
time_t CallbackBase::last_t() const { return _last_t; }
bool   CallbackBase::get_kill_me() const { return _kill_me; }
void   CallbackBase::kill_me() { _kill_me = true; }
void   CallbackBase::update_last_t() { _last_t = time( 0 ); }

/* ----------------------------- EventQueueBase ----------------------------- */

EventQueueBase::~EventQueueBase() {}

/* -------------------------------------------------------------------------- */

#ifdef __linux__

/* ------------------------------- EventQueue ------------------------------- */

EventQueue::EventQueue( int max_events ) : EventQueueBase( max_events ) {
    _epoll_fd = epoll_create( _max_events );
    if ( _epoll_fd == -1 ) { throw std::runtime_error( "epoll_create" ); }
    _events = new epoll_event[_max_events];
}

EventQueue::~EventQueue() {
    close( _epoll_fd );
    delete[] _events;
}

void EventQueue::add( int fd, const CallbackBase &callback ) {
    epoll_event event;
    event.data.fd = fd;
    event.events  = EPOLLIN | EPOLLOUT;
    if ( epoll_ctl( _epoll_fd, EPOLL_CTL_ADD, fd, &event ) == -1 ) {
        throw std::runtime_error( "epoll_ctl" );
    }
    _callbacks.insert(
        std::make_pair( fd, PolymorphicWrapper< CallbackBase >( callback ) ) );
}

void EventQueue::remove( int fd ) {
    if ( epoll_ctl( _epoll_fd, EPOLL_CTL_DEL, fd, 0 ) == -1 ) {
        throw std::runtime_error( "epoll_ctl" );
    }
    _callbacks.erase( fd );
    close( fd );
}

void EventQueue::wait() {
    int n_events = epoll_wait( _epoll_fd, _events, _max_events, 1000 );
    if ( n_events == -1 ) { throw std::runtime_error( "epoll_wait" ); }
    for ( int i = 0; i < n_events; i++ ) {
        if ( !_callbacks.count( _events[i].data.fd ) ) { continue; }
        if ( _callbacks.at( _events[i].data.fd )->get_kill_me() ) {
            remove( _events[i].data.fd );
        } else if ( _events[i].events & EPOLLIN ) {
            _callbacks.at( _events[i].data.fd )->handle_read();
        } else if ( _events[i].events & EPOLLOUT ) {
            _callbacks.at( _events[i].data.fd )->handle_write();
        }
    }
    for ( std::map< int, PolymorphicWrapper< CallbackBase > >::iterator it(
              _callbacks.begin() );
          it != _callbacks.end(); ) {
        std::map< int, PolymorphicWrapper< CallbackBase > >::iterator tmp( it );
        tmp++;
        if ( ( it->second->idle_to()
               && time( 0 ) - it->second->last_t() > it->second->idle_to() )
             || ( it->second->con_to()
                  && time( 0 ) - it->second->t0() > it->second->con_to() ) ) {
            it->second->handle_timeout();
        }
        it = tmp;
    }
}

/* -------------------------------------------------------------------------- */

#else

/* ------------------------------- EventQueue ------------------------------- */

EventQueue::EventQueue( int max_events ) : EventQueueBase( max_events ) {
    _kqueue_fd = kqueue();
    if ( _kqueue_fd == -1 ) { throw std::runtime_error( "kqueue" ); }
    _events = new struct kevent[_max_events];
}

EventQueue::~EventQueue() {
    close( _kqueue_fd );
    delete[] _events;
}

void EventQueue::add( int fd, const CallbackBase &callback ) {
    struct kevent events[2];
    EV_SET( events, fd, EVFILT_READ, EV_ADD, 0, 0, 0 );
    EV_SET( events + 1, fd, EVFILT_WRITE, EV_ADD, 0, 0, 0 );
    if ( kevent( _kqueue_fd, events, 2, 0, 0, 0 ) == -1 ) {
        throw std::runtime_error( "kevent" );
    }
    _callbacks.insert(
        std::make_pair( fd, PolymorphicWrapper< CallbackBase >( callback ) ) );
}

void EventQueue::remove( int fd ) {
    struct kevent events[2];
    EV_SET( events, fd, EVFILT_READ, EV_DELETE, 0, 0, 0 );
    EV_SET( events + 1, fd, EVFILT_WRITE, EV_DELETE, 0, 0, 0 );
    if ( kevent( _kqueue_fd, events, 2, 0, 0, 0 ) == -1 ) {
        throw std::runtime_error( "kevent" );
    }
    _callbacks.erase( fd );
    close( fd );
}

void EventQueue::wait() {
    struct timespec ts = { 1, 0 };
    int n_events       = kevent( _kqueue_fd, 0, 0, _events, _max_events, &ts );
    if ( n_events == -1 ) { throw std::runtime_error( "kevent" ); }
    for ( int i = 0; i < n_events; i++ ) {
        if ( !_callbacks.count( _events[i].ident ) ) { continue; }
        if ( _callbacks.at( _events[i].ident )->get_kill_me() ) {
            remove( _events[i].ident );
            continue;
        }
        switch ( _events[i].filter ) {
        case EVFILT_READ:
            _callbacks.at( _events[i].ident )->handle_read();
            break;
        case EVFILT_WRITE:
            _callbacks.at( _events[i].ident )->handle_write();
            break;
        }
    }
    for ( std::map< int, PolymorphicWrapper< CallbackBase > >::iterator it(
              _callbacks.begin() );
          it != _callbacks.end(); ) {
        std::map< int, PolymorphicWrapper< CallbackBase > >::iterator tmp( it );
        tmp++;
        if ( ( it->second->idle_to()
               && time( 0 ) - it->second->last_t() > it->second->idle_to() )
             || ( it->second->con_to()
                  && time( 0 ) - it->second->t0() > it->second->con_to() ) ) {
            it->second->handle_timeout();
        }
        it = tmp;
    }
}

/* -------------------------------------------------------------------------- */

#endif
