#include "Traits.hpp"

/* -------------------------------------------------------------------------- */

std::ostream &Trait::operator<<( std::ostream &os, const Trait::Repr &r ) {
    return r.repr( os );
}

/* -------------------------------------------------------------------------- */
