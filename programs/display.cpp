#include "display.hpp"
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>

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
    for (char& c : text) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return text;
}

std::string requiredField(const std::map<std::string, std::string>& fields,
                          const std::string& key,
                          const std::string& filename) {
    const auto it = fields.find(key);
    if (it == fields.end() || it->second.empty()) {
        throw std::runtime_error("Display: champ " + key + " manquant dans " + filename);
    }
    return it->second;
}

} // namespace

void Display::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Display: impossible d'ouvrir " + filename);
    }

    std::map<std::string, std::string> fields;
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
            throw std::runtime_error("Display: ligne invalide " + std::to_string(lineNumber)
                                     + " dans " + filename);
        }

        const std::string key = upper(trim(line.substr(0, separator)));
        const std::string value = trim(line.substr(separator + 1));
        if (key != "TYPE" && key != "REFRESH" && key != "SOURCE") {
            throw std::runtime_error("Display: champ inconnu '" + key + "' dans " + filename);
        }
        if (fields.count(key) != 0) {
            throw std::runtime_error("Display: champ " + key + " en double dans " + filename);
        }
        fields.emplace(key, value);
    }

    if (upper(requiredField(fields, "TYPE", filename)) != "DISPLAY") {
        throw std::runtime_error("Display: TYPE doit etre DISPLAY dans " + filename);
    }

    const std::string newSourceLabel = requiredField(fields, "SOURCE", filename);
    const std::string refreshText = requiredField(fields, "REFRESH", filename);
    size_t parsedLength = 0;
    int newRefresh = 0;
    try {
        newRefresh = std::stoi(refreshText, &parsedLength);
    } catch (const std::exception&) {
        throw std::runtime_error("Display: REFRESH invalide dans " + filename);
    }
    if (parsedLength != refreshText.size() || newRefresh <= 0) {
        throw std::runtime_error("Display: REFRESH doit etre un entier strictement positif dans " + filename);
    }

    sourceLabel = newSourceLabel;
    refresh = newRefresh;
    source = nullptr;
    stepCount = 0;
}

void Display::bind(Readable* newSource) {
    if (newSource == nullptr) {
        throw std::invalid_argument("Display: source nulle");
    }
    if (sourceLabel.empty()) {
        throw std::logic_error("Display: charger le fichier avant bind()");
    }
    if (newSource->getLabel() != sourceLabel) {
        throw std::invalid_argument("Display: le label de la source ne correspond pas a SOURCE: "
                                    + sourceLabel);
    }
    source = newSource;
}

void Display::simulate() {
    if (source == nullptr) {
        throw std::logic_error("Display: source non liee (bind manquant)");
    }

    ++stepCount;
    if (stepCount < refresh) {
        return;
    }
    stepCount = 0;

    while (true) {
        const DataValue data = source->read();
        if (!data.valid) {
            break;
        }
        std::cout << data.value << std::endl;
    }
}
