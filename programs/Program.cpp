#include "Program.hpp"
#include <fstream>
#include <iostream>

// Charge le fichier d'instructions texte et construit la liste du programme
void Program::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    
    // Vérification de l'ouverture du fichier
    if (!file.is_open()) {
        std::cerr << "Erreur : Impossible d'ouvrir le fichier de programme " << filename << std::endl;
        return;
    }

    std::string opStr;
    // Lecture mot par mot (chaque instruction commence par un code d'opération)
    while (file >> opStr) {
        if (opStr == "NOP") {
            // emplace_back (méthode de std::vector)
            //   1 : Le vector alloue directement la case mémoire à la fin du tableau. 
            //   2 : Il construit l'objet Instruction directement à cet emplacement en utilisant les arguments fournis (OpCode::ADD, o1, o2).

            // L'instruction NOP n'a pas d'opérandes
            instructions.emplace_back(OpCode::NOP);
        } else {
            double o1, o2;
            // Lecture des deux opérandes numériques
            file >> o1 >> o2;

            // Ajout de l'instruction correspondante dans le tableau
            if (opStr == "ADD")      instructions.emplace_back(OpCode::ADD, o1, o2);
            else if (opStr == "SUB") instructions.emplace_back(OpCode::SUB, o1, o2);
            else if (opStr == "MUL") instructions.emplace_back(OpCode::MUL, o1, o2);
            else if (opStr == "DIV") instructions.emplace_back(OpCode::DIV, o1, o2);
        }
    }

    // Remet le pointeur d'instruction au début (index 0)
    reset();
}

// Renvoie l'instruction courante puis avance le pointeur (Instruction Pointer)
Instruction Program::compute() {
    // Si toutes les instructions ont été exécutées, on renvoie NOP par sécurité
    if (isFinished()) {
        return Instruction(OpCode::NOP);
    }
    // Renvoie l'instruction à l'index 'ptr' puis incrémente 'ptr'
    return instructions[ptr++];
}

// Indique si le pointeur a dépassé la dernière instruction
bool Program::isFinished() const {
    return ptr >= instructions.size();
}

// Rembobine le pointeur d'instruction au début
void Program::reset() {
    ptr = 0;
}