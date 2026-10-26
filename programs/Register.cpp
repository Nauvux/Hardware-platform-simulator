#include "Register.hpp"

bool Register::isEmpty() const {
    return fifo.empty();
}

void Register::write(double val) {
    fifo.push(val);
}

double Register::read() {
    if (isEmpty()) return 0.0;
    double val = fifo.front();
    fifo.pop();
    return val;
}
