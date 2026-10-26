#ifndef REGISTER_HPP
#define REGISTER_HPP

#include <queue>

class Register {
private:
    std::queue<double> fifo;

public:
    Register() = default;
    
    bool isEmpty() const;
    void write(double val);
    double read();
};

#endif // REGISTER_HPP
