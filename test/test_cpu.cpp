// Tests de la partie "moteur" : Instruction, Register, Program, CPU.
// Lancement : make test
#include <cassert>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include "CPU.hpp"

using namespace std;

// Écrit un petit fichier de test dans le dossier courant
static string writeFile(const string& name, const string& content) {
    ofstream(name) << content;
    return name;
}

// Vrai si 'f' lève une exception du type E
template <class E, class F>
static bool throws(F f) {
    try {
        f();
    } catch (const E&) {
        return true;
    }
    return false;
}

static void testInstruction() {
    assert(Instruction(OpCode::ADD, 2, 3).compute() == 5);
    assert(fabs(Instruction(OpCode::SUB, 4.5, 1.2).compute() - 3.3) < 1e-12);
    assert(Instruction(OpCode::MUL, 2, 3).compute() == 6);
    assert(Instruction(OpCode::DIV, 8, 2).compute() == 4);
    assert(throws<runtime_error>([] { Instruction(OpCode::DIV, 1, 0).compute(); }));
    assert(Instruction().compute() == 0);                   // NOP
}

static void testRegister() {
    Register r;
    assert(r.isEmpty());
    r.write(1);
    r.write(2);
    assert(r.read() == 1); // FIFO : le premier écrit est le premier lu
    assert(r.read() == 2);
    assert(r.isEmpty());
    assert(throws<logic_error>([&] { r.read(); }));
}

static void testProgram() {
    Program p;
    p.loadFromFile(writeFile("t_prog.txt", "ADD 2 3\n\n  SUB   4.5  1.2  \nNOP\n"));
    assert(p.size() == 3); // la ligne vide est ignorée
    assert(p.compute().compute() == 5);
    p.compute();
    p.compute();
    assert(p.isFinished());
    assert(p.compute().compute() == 0); // fini -> NOP
    p.reset();
    assert(!p.isFinished());

    // Recharger ne doit pas ajouter à la suite
    p.loadFromFile("t_prog.txt");
    assert(p.size() == 3);

    // Erreurs
    assert(throws<runtime_error>([] { Program q; q.loadFromFile("absent.txt"); }));
    assert(throws<runtime_error>([] { Program q; q.loadFromFile(writeFile("t_bad1.txt", "JMP 1 2\n")); }));
    assert(throws<runtime_error>([] { Program q; q.loadFromFile(writeFile("t_bad2.txt", "MUL 2 x\n")); }));
    assert(throws<runtime_error>([] { Program q; q.loadFromFile(writeFile("t_bad3.txt", "ADD 1\n")); }));
    assert(throws<runtime_error>([] { Program q; q.loadFromFile(writeFile("t_bad4.txt", "ADD 1 2 3\n")); }));
    assert(throws<runtime_error>([] { Program q; q.loadFromFile(writeFile("t_bad5.txt", "DIV 4 0\n")); }));
}

static void testCpuLoading() {
    writeFile("t_prog2.txt", "ADD 1 1\nADD 2 2\n");
    CPU cpu;
    // Espaces en trop et "FREQUENCY:3" sans espace : doivent être lus correctement
    cpu.loadFromFile(writeFile("t_cpu.txt", "TYPE :  CPU\n\n  LABEL :  Main unit\nCORES: 2\nFREQUENCY:3\nPROGRAM: t_prog2.txt\n"));
    assert(cpu.getLabel() == "Main unit");
    assert(cpu.getCores() == 2);
    assert(cpu.getFrequency() == 3);

    assert(throws<runtime_error>([] { CPU c; c.loadFromFile("absent.txt"); }));
    assert(throws<runtime_error>([] {
        CPU c; c.loadFromFile(writeFile("t_cpu2.txt", "TYPE: CPU\nLABEL: x\nCORES: 0\nFREQUENCY: 1\nPROGRAM: t_prog2.txt\n"));
    }));
    assert(throws<runtime_error>([] {
        CPU c; c.loadFromFile(writeFile("t_cpu3.txt", "TYPE: CPU\nLABEL: x\nCORES: 1\nPROGRAM: t_prog2.txt\n")); // FREQUENCY manquant
    }));
    assert(throws<runtime_error>([] {
        CPU c; c.loadFromFile(writeFile("t_cpu4.txt", "TYPE: CPU\nLABEL x\n")); // ':' manquant
    }));
}

static void testCpuSimulation() {
    // Programme de 2 instructions, 2 cœurs, 3 instructions par pas
    CPU cpu;
    cpu.loadFromFile("t_cpu.txt");
    cpu.simulate(); // cœur 0 : 2, 4 ; programme fini -> cœur 1 : 2
    assert(cpu.getActiveCore() == 1);
    cpu.simulate(); // cœur 1 : 4, puis plus rien
    cpu.simulate(); // dernier cœur terminé : ne fait plus rien
    const double expected[] = {2, 4, 2, 4};
    for (double e : expected) {
        DataValue v = cpu.read();
        assert(v.valid && v.value == e);
    }
    assert(!cpu.read().valid); // registre vide -> valeur invalide

    // Programme vide : aucun résultat (pas de NOP = 0 dans le registre)
    writeFile("t_empty.txt", "\n");
    CPU empty;
    empty.loadFromFile(writeFile("t_cpu5.txt", "TYPE: CPU\nLABEL: e\nCORES: 3\nFREQUENCY: 4\nPROGRAM: t_empty.txt\n"));
    empty.simulate();
    assert(!empty.read().valid);
}

static void cleanUp() {
    const char* files[] = {"t_prog.txt", "t_prog2.txt", "t_empty.txt", "t_bad1.txt", "t_bad2.txt",
                           "t_bad3.txt", "t_bad4.txt", "t_bad5.txt", "t_cpu.txt", "t_cpu2.txt",
                           "t_cpu3.txt", "t_cpu4.txt", "t_cpu5.txt"};
    for (const char* f : files) {
        remove(f);
    }
}

int main() {
    cout << "--- Tests du module CPU ---" << endl;
    testInstruction();
    testRegister();
    testProgram();
    testCpuLoading();
    testCpuSimulation();
    cleanUp();
    cout << "Tous les tests passent." << endl;
    return 0;
}
