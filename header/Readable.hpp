#ifndef READABLE_HPP
#define READABLE_HPP

#include "DataValue.hpp"

// Interface : "quelque chose que l'on peut lire" (CPU, Bus, Memory).
// getLabel() a été déplacé dans Component, car le Display a aussi un label
// alors qu'il n'est pas lisible.
class Readable {
public:
    virtual ~Readable() = default;
    virtual DataValue read() = 0;
};

#endif // READABLE_HPP
