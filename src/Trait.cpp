#include "Trait.hpp"

/* ---------------------------------- Repr ---------------------------------- */

std::ostream &Trait::operator<<( std::ostream &os, const Trait::Repr &r ) {
    return r.repr( os );
}

/* -------------------------------------------------------------------------- */
