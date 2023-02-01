#pragma once

#include "common.h"

namespace Trait {

/* -------------------------------- CloneCRTP ------------------------------- */

template < typename T > struct CloneCRTP { virtual T *clone() const = 0; };

/* -------------------------------- Stringify ------------------------------- */

struct Stringify {
    virtual std::string stringify() const = 0;
};

/* ---------------------------------- Repr ---------------------------------- */

struct Repr {
    virtual std::ostream &repr( std::ostream &os ) const = 0;
};

std::ostream &operator<<( std::ostream &os, const Repr &r );

/* -------------------------------------------------------------------------- */

}
