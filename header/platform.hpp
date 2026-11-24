#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include <memory>
#include <string>
#include <vector>
#include "CPU.hpp"
#include "bus.hpp"
#include "memory.hpp"
#include "display.hpp"

// La plateforme possede les composants et organise leurs 3 phases.
class Platform {
private:
    std::vector<std::unique_ptr<CPU>> cpus; //unique signifie qu'un objet ne peut être possédé que par un seul unique_ptr à la fois.
    std::vector<std::unique_ptr<Bus>> buses;
    std::vector<std::unique_ptr<Memory>> memories;
    std::vector<std::unique_ptr<Display>> displays;
    bool bound = false;

public:
    Platform() = default;

    // Phase 1 : cree les composants listes dans platform.txt.
    void loadFromFile(const std::string& filename);

    // Phase 2 : relie chaque Bus, Memory et Display a sa source Readable.
    void bind();

    // Phase 3 : effectue un cycle, ou plusieurs cycles successifs.
    void simulate();
    void simulate(int steps);
};

#endif // PLATFORM_HPP
