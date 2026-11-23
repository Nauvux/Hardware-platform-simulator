#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

#include <map>
#include <string>

// Lecture d'un fichier de description "MOT-CLÉ: valeur".
// Écrit pour le CPU, mais réutilisable tel quel pour Bus, Memory et Display.
//
// Tolère les espaces en trop ("TYPE :   CPU", "FREQUENCY:3") et les lignes vides.
// Lève std::runtime_error si le fichier est absent ou si une ligne n'a pas de ':'.
class ConfigFile {
private:
    std::string filename;
    std::map<std::string, std::string> values; // mot-clé (en majuscules) -> valeur

public:
    explicit ConfigFile(const std::string& filename);

    bool has(const std::string& key) const;

    // Valeur d'un mot-clé obligatoire (lève une exception s'il manque)
    std::string get(const std::string& key) const;

    // Entier obligatoire, vérifié >= minimum
    int getInt(const std::string& key, int minimum) const;
};

// Retire les espaces et tabulations au début et à la fin
std::string trim(const std::string& s);

#endif // CONFIGFILE_HPP
