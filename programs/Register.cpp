// Register.cpp
#include "Register.hpp"
#include <stdexcept>

using namespace std;

bool Register::isEmpty() const {
    return fifo.empty();
}

void Register::write(double val) {
    fifo.push(val); // Ajoute en fin de file
}

double Register::read() {
    // Lire un registre vide est une erreur de programmation : on la signale
    // au lieu de renvoyer 0, qui ressemblerait à un vrai résultat.
    // (Le CPU vérifie toujours isEmpty() avant d'appeler read().)
    if (isEmpty()) {
        throw logic_error("Register::read() sur un registre vide");
    }
    double val = fifo.front(); // Récupère le premier élément entré
    fifo.pop();                // Retire cet élément de la file
    return val;
}
