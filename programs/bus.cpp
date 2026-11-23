#include "bus.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

namespace {

std::string trim(const std::string& text) {
    const std::string whitespace = " \t\r\n";
    const std::string::size_type begin = text.find_first_not_of(whitespace);
    if (begin == std::string::npos) {
        return "";
    }
    const std::string::size_type end = text.find_last_not_of(whitespace);
    return text.substr(begin, end - begin + 1);
}

std::string upper(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return text;
}

std::string requiredField(const std::unordered_map<std::string, std::string>& fields,
                          const std::string& key,
                          const std::string& filename) {
    const auto it = fields.find(key);
    if (it == fields.end() || it->second.empty()) {
        throw std::runtime_error("Bus: champ " + key + " manquant dans " + filename);
    }
    return it->second;
}

} // namespace

void Bus::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Bus: impossible d'ouvrir " + filename);
    }

    std::unordered_map<std::string, std::string> fields;
    std::string line;
    size_t lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        const std::string::size_type separator = line.find(':');
        if (separator == std::string::npos) {
            throw std::runtime_error("Bus: ligne invalide " + std::to_string(lineNumber)
                                     + " dans " + filename);
        }

        const std::string key = upper(trim(line.substr(0, separator)));
        const std::string value = trim(line.substr(separator + 1));
        if (key != "TYPE" && key != "LABEL" && key != "WIDTH" && key != "SOURCE") {
            throw std::runtime_error("Bus: champ inconnu '" + key + "' dans " + filename);
        }
        if (fields.count(key) != 0) {
            throw std::runtime_error("Bus: champ " + key + " en double dans " + filename);
        }
        fields.emplace(key, value);
    }

    if (upper(requiredField(fields, "TYPE", filename)) != "BUS") {
        throw std::runtime_error("Bus: TYPE doit etre BUS dans " + filename);
    }

    const std::string newLabel = requiredField(fields, "LABEL", filename);
    const std::string newSourceLabel = requiredField(fields, "SOURCE", filename);
    const std::string widthText = requiredField(fields, "WIDTH", filename);

    size_t parsedLength = 0;
    int newWidth = 0;
    try {
        newWidth = std::stoi(widthText, &parsedLength);
    } catch (const std::exception&) {
        throw std::runtime_error("Bus: WIDTH invalide dans " + filename);
    }
    if (parsedLength != widthText.size() || newWidth <= 0) {
        throw std::runtime_error("Bus: WIDTH doit etre un entier strictement positif dans " + filename);
    }

    // On ne modifie l'etat que lorsque le fichier entier est valide.
    label = newLabel;
    sourceLabel = newSourceLabel;
    width = newWidth;
    source = nullptr; // Un nouveau chargement necessite un nouveau bind().
    pending = std::queue<DataValue>();

