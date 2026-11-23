#include "CPU.hpp"
#include "ConfigFile.hpp"
#include <stdexcept>

using namespace std;

// Lit le fichier de configuration du CPU et instancie ses paramètres.
// Lève std::runtime_error si un mot-clé manque ou si une valeur est invalide.
void CPU::loadFromFile(const std::string& filename) {
    ConfigFile config(filename);

    if (config.get("TYPE") != "CPU") {
        throw std::runtime_error(filename + " : TYPE devrait être CPU");
    }
    label = config.get("LABEL");
    cores = config.getInt("CORES", 1);         // au moins 1 cœur
    frequency = config.getInt("FREQUENCY", 1); // au moins 1 instruction par pas

    // Charge immédiatement le fichier de programme associé
    program.loadFromFile(config.get("PROGRAM"));

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
            }
            // Tous les cœurs ont terminé (ou le programme est vide) : on arrête.
            // Sans ce 2e test, un programme vide écrirait des NOP = 0 dans le registre.
            if (program.isFinished()) {
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
