#ifndef READABLE_HPP
#define READABLE_HPP

#include <string>
#include "DataValue.hpp"

class Readable {
public:
    virtual ~Readable() = default;
    virtual DataValue read() = 0;
    virtual std::string getLabel() const = 0;
};

#endif // READABLE_HPP