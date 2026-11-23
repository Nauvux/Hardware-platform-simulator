#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <string>

// Classe de base commune à TOUS les composants (CPU, Bus, Memory, Display).
// C'est elle que la Platform manipule : elle appelle simulate() sur chaque
// composant sans connaître son type exact.
//
// Contrat avec la partie "infrastructure" (Bus, Memory, Display, Platform) :
//   - tout composant hérite de Component et a un label ;
//   - les composants qu'on peut lire héritent EN PLUS de Readable.
class Component {
protected:
    std::string label;

public:
    virtual ~Component() = default;

    std::string getLabel() const { return label; }

    // Un pas de simulation
    virtual void simulate() = 0;
};

#endif // COMPONENT_HPP
