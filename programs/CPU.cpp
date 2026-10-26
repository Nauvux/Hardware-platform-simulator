#include "CPU.hpp"
#include <fstream>
#include <iostream>

// Lit le fichier de configuration du CPU et instancie ses paramètres
void CPU::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Erreur : Impossible d'ouvrir le fichier CPU " << filename << std::endl;
        return;
    }

    std::string key;
    // Parcours du fichier par mot-clé (KEYWORD:)
    while (file >> key) {
        if (key == "TYPE:") {
            std::string type;
            file >> type; // Lit "CPU" (ignoré car déjà dans la classe CPU)
        } else if (key == "LABEL:") {
            // Lit tout le reste de la ligne pour conserver le nom complet (ex: "Main CPU")
            std::getline(file >> std::ws, label);
        } else if (key == "CORES:") {
            file >> cores;
        } else if (key == "FREQUENCY:") {
            file >> frequency;
        } else if (key == "PROGRAM:") {
            std::string progPath;
            file >> progPath;
            // Charge immédiatement le fichier de programme associé
            program.loadFromFile(progPath);
        }
    }

    // On s'assure que le premier cœur (index 0) est actif au démarrage
    activeCore = 0;
}

// Effectue un cycle de simulation du CPU (exécuté à chaque étape globale)
void CPU::simulate() {
    // Le CPU exécute jusqu'à F (frequency) instructions par étape
    for (int i = 0; i < frequency; ++i) {

        // Si le programme est terminé sur le cœur actuel
        if (program.isFinished()) {
            // S'il reste des cœurs inactifs, on passe au suivant
            if (activeCore < cores - 1) {
                activeCore++;
                program.reset(); // On réinitialise le programme pour le nouveau cœur
            } else {
                // Tous les cœurs ont terminé leurs instructions : on arrête les calculs
                break;
            }
        }

        // Récupère l'instruction courante, calcule le résultat et l'écrit dans le registre FIFO
        Instruction inst = program.compute();
        double result = inst.compute();
        reg.write(result);
    }
}

// Méthode de l'interface Readable : appelée par le Bus pour lire une valeur
DataValue CPU::read() {
    // Si le registre est vide, la valeur renvoyée est invalide (valid = false)
    if (reg.isEmpty()) {
        return {0.0, false};
    }
    // Sinon, on dépile la plus ancienne valeur disponible (valid = true)
    return {reg.read(), true};
}