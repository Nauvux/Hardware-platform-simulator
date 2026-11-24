#ifndef BINDABLE_HPP
#define BINDABLE_HPP

#include <string>
#include "Readable.hpp"

// Interface : composant qui se branche sur une source lisible (Bus, Memory, Display).
// La Platform s'en sert en phase 2 sans connaître le type exact du composant.
class Bindable {
public:
    virtual ~Bindable() = default;
    virtual std::string getSourceLabel() const = 0;
    virtual void bind(Readable* newSource) = 0;
};

#endif // BINDABLE_HPP
