// Point d'entrée du simulateur.
// Usage : ./simulator <fichier_plateforme> <nombre_de_pas>
//   ex. : ./simulator testdata/platform.txt 20
#include <iostream>
#include <stdexcept>
#include <string>
#include "platform.hpp"

using namespace std;

static void printUsage(const char* program) {
    cerr << "Usage : " << program << " <fichier_plateforme> <nombre_de_pas>" << endl;
    cerr << "  ex. : " << program << " testdata/platform.txt 20" << endl;
}

// Convertit le nombre de pas passé en argument ; refuse "abc", "12x" et les négatifs.
static int parseSteps(const string& text) {
    size_t used = 0;
    int steps = 0;
    try {
        steps = stoi(text, &used);
    } catch (const exception&) {
        used = 0;
    }
    if (used == 0 || used != text.size() || steps < 0) {
        throw invalid_argument("nombre de pas invalide : \"" + text + "\" (entier >= 0 attendu)");
    }
    return steps;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printUsage(argv[0]);
        return 2;
    }

    try {
        const int steps = parseSteps(argv[2]);

        Platform platform;
        platform.loadFromFile(argv[1]); // Phase 1 : chargement de tous les composants
        platform.bind();                // Phase 2 : branchement de chaque composant à sa source
        platform.simulate(steps);       // Phase 3 : simulation pas à pas
    } catch (const exception& e) {
        // Toutes les erreurs (fichier absent, champ invalide, source introuvable...)
        // remontent jusqu'ici : on affiche le message au lieu de planter.
        cerr << "Erreur : " << e.what() << endl;
        return 1;
    }
    return 0;
}
