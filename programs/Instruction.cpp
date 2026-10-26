// Instruction.cpp
#include "Instruction.hpp"

Instruction::Instruction(OpCode code, double o1, double o2)
    : opcode(code), op1(o1), op2(o2) {}

// Unité Arithmétique et Logique (ALU)
double Instruction::compute() const {
    switch (opcode) {
        case OpCode::ADD: 
            return op1 + op2;
        case OpCode::SUB: 
            return op1 - op2;
        case OpCode::MUL: 
            return op1 * op2;
        case OpCode::DIV: 
            return (op2 != 0.0) ? (op1 / op2) : 0.0; // Protection contre la division par zéro
        case OpCode::NOP:
        default:          
            return 0.0;
    }
}