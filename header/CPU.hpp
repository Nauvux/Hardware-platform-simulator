#ifndef CPU_HPP
#define CPU_HPP

#include <string>
#include "Component.hpp"
#include "Readable.hpp"
#include "Register.hpp"
#include "Program.hpp"

// Le CPU est un composant (simulate) qu'on peut lire (read)
class CPU : public Component, public Readable {
private:
    int cores = 1;
    int activeCore = 0;
    int frequency = 1;
    Program program;
    Register reg;

public:
    CPU() = default;

    // Lève std::runtime_error si le fichier (ou son programme) est invalide
    void loadFromFile(const std::string& filename);
    void simulate() override;

    // Implémentation de l'interface Readable
    DataValue read() override;

    int getActiveCore() const { return activeCore; }
    int getCores() const { return cores; }
    int getFrequency() const { return frequency; }
};

#endif // CPU_HPP
