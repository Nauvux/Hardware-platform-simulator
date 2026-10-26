#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <vector>
#include <string>
#include "Instruction.hpp"

class Program {
private:
    std::vector<Instruction> instructions;
    size_t ptr = 0;

public:
    Program() = default;

    void loadFromFile(const std::string& filename);
    Instruction compute();
    bool isFinished() const;
    void reset();
};

#endif // PROGRAM_HPP
