#ifndef CPU_HPP
#define CPU_HPP

#include <string>
#include "Readable.hpp"
#include "Register.hpp"
#include "Program.hpp"

class CPU : public Readable {
private:
    std::string label;
    int cores = 1;
    int activeCore = 0;
    int frequency = 1;
    Program program;
    Register reg;

public:
    CPU() = default;

    void loadFromFile(const std::string& filename);
    void simulate();
    
    // Implémentation de l'interface Readable
    DataValue read() override;
    std::string getLabel() const override { return label; }

    int getActiveCore() const { return activeCore; }
};

#endif // CPU_HPP