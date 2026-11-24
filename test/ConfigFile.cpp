#include "ConfigFile.hpp"

#include <cctype>
#include <fstream>
#include <stdexcept>

using namespace std;

string trim(const string& s) {
    const string blanks = " \t\r\n";
    size_t first = s.find_first_not_of(blanks);
    if (first == string::npos) {
        return "";
    }
    size_t last = s.find_last_not_of(blanks);
    return s.substr(first, last - first + 1);
}

ConfigFile::ConfigFile(const string& name) : filename(name) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("impossible d'ouvrir le fichier " + filename);
    }

    string line;
    int lineNumber = 0;
    // Lecture LIGNE par ligne (et pas mot par mot) : on découpe sur le premier ':'
    while (getline(file, line)) {
        lineNumber++;
        const string content = trim(line);
        if (content.empty() || content[0] == '#') {
            continue; // ligne vide ou commentaire
        }
        size_t colon = line.find(':');
        if (colon == string::npos) {
            throw runtime_error(filename + ":" + to_string(lineNumber)
                                     + " : ':' manquant dans \"" + trim(line) + "\"");
        }
        string key = trim(line.substr(0, colon));
        string value = trim(line.substr(colon + 1));
        for (char& c : key) {
            c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
        }
        values[key] = value;
    }
}

bool ConfigFile::has(const string& key) const {
    return values.count(key) != 0;
}

string ConfigFile::get(const string& key) const {
    auto it = values.find(key);
    if (it == values.end() || it->second.empty()) {
        throw runtime_error(filename + " : mot-clé " + key + " manquant");
    }
    return it->second;
}

int ConfigFile::getInt(const string& key, int minimum) const {
    string text = get(key);
    size_t used = 0;
    int value = 0;
    try {
        value = stoi(text, &used);
    } catch (const exception&) {
        used = 0;
    }
    if (used == 0 || used != text.size()) {
        throw runtime_error(filename + " : " + key + " doit être un entier (\"" + text + "\")");
    }
    if (value < minimum) {
        throw runtime_error(filename + " : " + key + " doit être >= " + to_string(minimum));
    }
    return value;
}
