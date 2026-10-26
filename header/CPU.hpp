#ifndef CPU_HPP
#define CPU_HPP

#include <string>
#include "DataValue.hpp"
#include "Register.hpp"
#include "Program.hpp"

class CPU {
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
    DataValue read();

    // Getters pour de futurs tests ou affichages
    const std::string& getLabel() const { return label; }
    int getActiveCore() const { return activeCore; }
};

#endif // CPU_HPP
