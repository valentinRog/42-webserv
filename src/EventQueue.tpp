#include "EventQueue.hpp"

/* -------------------------------------------------------------------------- */

template < class F >
Callback< F >::Callback( F f, time_t con_to, time_t idle_to )
    : CallbackBase( con_to, idle_to ),
      _f( f ) {}

template < class F > void Callback< F >::operator()() { _f(); }

/* -------------------------------------------------------------------------- */
