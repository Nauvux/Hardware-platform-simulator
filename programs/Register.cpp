// Register.cpp
#include "Register.hpp"

bool Register::isEmpty() const {
    return fifo.empty();
}

void Register::write(double val) {
    fifo.push(val); // Ajoute en fin de file
}

double Register::read() {
    if (isEmpty()) return 0.0;
    double val = fifo.front(); // Récupère le premier élément entré
    fifo.pop();                // Retire cet élément de la file
    return val;
}