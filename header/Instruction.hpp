#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

enum class OpCode { NOP, ADD, SUB, MUL, DIV };

class Instruction {
private:
    OpCode opcode;
    double op1;
    double op2;

public:
    Instruction(OpCode code = OpCode::NOP, double o1 = 0.0, double o2 = 0.0);
    double compute() const;
};

#endif // INSTRUCTION_HPP
