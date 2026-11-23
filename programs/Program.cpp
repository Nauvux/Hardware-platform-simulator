#include "Program.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

// Charge le fichier d'instructions texte et construit la liste du programme.
// Lève std::runtime_error (avec le numéro de ligne) si le fichier est absent
// ou si une instruction est invalide.
void Program::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);

    // Vérification de l'ouverture du fichier
    if (!file.is_open()) {
        throw std::runtime_error("impossible d'ouvrir le fichier de programme " + filename);
    }

    // On repart d'un programme vide (sinon un 2e appel ajouterait à la suite)
    instructions.clear();

    std::string line;
    int lineNumber = 0;
    // Lecture ligne par ligne : une instruction par ligne
    while (std::getline(file, line)) {
        lineNumber++;
        std::istringstream in(line);
        std::string opStr;
        if (!(in >> opStr)) {
            continue; // ligne vide
        }
        std::string where = filename + ":" + std::to_string(lineNumber) + " : ";

        if (opStr == "NOP") {
            // emplace_back (méthode de std::vector)
            //   1 : Le vector alloue directement la case mémoire à la fin du tableau.
            //   2 : Il construit l'objet Instruction directement à cet emplacement.

            // L'instruction NOP n'a pas d'opérandes
            instructions.emplace_back(OpCode::NOP);
            continue;
        }

        OpCode code;
        if (opStr == "ADD")      code = OpCode::ADD;
        else if (opStr == "SUB") code = OpCode::SUB;
        else if (opStr == "MUL") code = OpCode::MUL;
        else if (opStr == "DIV") code = OpCode::DIV;
        else {
            throw std::runtime_error(where + "instruction inconnue \"" + opStr + "\"");
        }

        // Exactement deux opérandes numériques, rien d'autre sur la ligne
        double o1, o2;
        std::string extra;
        if (!(in >> o1 >> o2)) {
            throw std::runtime_error(where + opStr + " attend deux nombres");
        }
        if (in >> extra) {
            throw std::runtime_error(where + "texte en trop \"" + extra + "\"");
        }

        // Les opérandes sont des constantes : une division par zéro se voit dès le chargement
        if (code == OpCode::DIV && o2 == 0.0) {
            throw std::runtime_error(where + "division par zéro");
        }

        // Ajout de l'instruction correspondante dans le tableau
        instructions.emplace_back(code, o1, o2);
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

size_t Program::size() const {
    return instructions.size();
}
