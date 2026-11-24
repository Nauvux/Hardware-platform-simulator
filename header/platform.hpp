#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include <memory>
#include <string>
#include <vector>
#include "Component.hpp"

// La plateforme possede les composants et organise leurs 3 phases.
// Elle ne les connait qu'a travers Component (polymorphisme) : ajouter un
// nouveau type de composant ne demande de modifier que loadFromFile().
class Platform {
private:
    // unique_ptr : un objet ne peut etre possede que par un seul unique_ptr a la fois.
    std::vector<std::unique_ptr<Component>> components;
    bool bound = false;

public:
    Platform() = default;

    // Phase 1 : cree les composants listes dans platform.txt.
    void loadFromFile(const std::string& filename);

    // Phase 2 : relie chaque composant Bindable a sa source Readable.
    void bind();

    // Phase 3 : effectue un cycle, ou plusieurs cycles successifs.
    void simulate();
    void simulate(int steps);
};

#endif // PLATFORM_HPP
