#pragma once

#include "common.h"

namespace Trait {

/* -------------------------------------------------------------------------- */

template < typename T > struct CloneCRTP { virtual T *clone() const = 0; };

/* -------------------------------------------------------------------------- */

struct Stringify {
    virtual std::string stringify() const = 0;
};

/* -------------------------------------------------------------------------- */

struct Repr {
    virtual std::ostream &repr( std::ostream &os ) const = 0;
};

std::ostream &operator<<( std::ostream &os, const Repr &r );

/* -------------------------------------------------------------------------- */

}
