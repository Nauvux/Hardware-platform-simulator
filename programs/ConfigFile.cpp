#include "ConfigFile.hpp"

#include <cctype>
#include <fstream>
#include <stdexcept>

std::string trim(const std::string& s) {
    const std::string blanks = " \t\r\n";
    size_t first = s.find_first_not_of(blanks);
    if (first == std::string::npos) {
        return "";
    }
    size_t last = s.find_last_not_of(blanks);
    return s.substr(first, last - first + 1);
}

ConfigFile::ConfigFile(const std::string& name) : filename(name) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("impossible d'ouvrir le fichier " + filename);
    }

    std::string line;
    int lineNumber = 0;
    // Lecture LIGNE par ligne (et pas mot par mot) : on découpe sur le premier ':'
    while (std::getline(file, line)) {
        lineNumber++;
        if (trim(line).empty()) {
            continue; // ligne vide
        }
        size_t colon = line.find(':');
        if (colon == std::string::npos) {
            throw std::runtime_error(filename + ":" + std::to_string(lineNumber)
                                     + " : ':' manquant dans \"" + trim(line) + "\"");
        }
        std::string key = trim(line.substr(0, colon));
        std::string value = trim(line.substr(colon + 1));
        for (char& c : key) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        values[key] = value;
    }
}

bool ConfigFile::has(const std::string& key) const {
    return values.count(key) != 0;
}

std::string ConfigFile::get(const std::string& key) const {
    auto it = values.find(key);
    if (it == values.end() || it->second.empty()) {
        throw std::runtime_error(filename + " : mot-clé " + key + " manquant");
    }
    return it->second;
}

int ConfigFile::getInt(const std::string& key, int minimum) const {
    std::string text = get(key);
    size_t used = 0;
    int value = 0;
    try {
        value = std::stoi(text, &used);
    } catch (const std::exception&) {
        used = 0;
    }
    if (used == 0 || used != text.size()) {
        throw std::runtime_error(filename + " : " + key + " doit être un entier (\"" + text + "\")");
    }
    if (value < minimum) {
        throw std::runtime_error(filename + " : " + key + " doit être >= " + std::to_string(minimum));
    }
    return value;
}
