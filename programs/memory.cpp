#include "memory.hpp"
#include <cctype>
#include <fstream>
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
        throw std::runtime_error("Memory: champ " + key + " manquant dans " + filename);
    }
    return it->second;
}

int positiveInteger(const std::string& text, const std::string& key,
                    const std::string& filename) {
    size_t parsedLength = 0;
    int result = 0;
    try {
        result = std::stoi(text, &parsedLength);
    } catch (const std::exception&) {
        throw std::runtime_error("Memory: " + key + " invalide dans " + filename);
    }
    if (parsedLength != text.size() || result <= 0) {
        throw std::runtime_error("Memory: " + key
                                 + " doit etre un entier strictement positif dans " + filename);
    }
    return result;
}

} // namespace

void Memory::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Memory: impossible d'ouvrir " + filename);
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
            throw std::runtime_error("Memory: ligne invalide " + std::to_string(lineNumber)
                                     + " dans " + filename);
        }

        const std::string key = upper(trim(line.substr(0, separator)));
        const std::string value = trim(line.substr(separator + 1));
        if (key != "TYPE" && key != "LABEL" && key != "SIZE"
            && key != "ACCESS" && key != "SOURCE") {
            throw std::runtime_error("Memory: champ inconnu '" + key + "' dans " + filename);
        }
        if (fields.count(key) != 0) {
            throw std::runtime_error("Memory: champ " + key + " en double dans " + filename);
        }
        fields.emplace(key, value);
    }

    if (upper(requiredField(fields, "TYPE", filename)) != "MEMORY") {
        throw std::runtime_error("Memory: TYPE doit etre MEMORY dans " + filename);
    }

    const std::string newLabel = requiredField(fields, "LABEL", filename);
    const std::string newSourceLabel = requiredField(fields, "SOURCE", filename);
    const int newSize = positiveInteger(requiredField(fields, "SIZE", filename), "SIZE", filename);
    const int newAccess = positiveInteger(requiredField(fields, "ACCESS", filename), "ACCESS", filename);

    std::vector<double> newValues(static_cast<size_t>(newSize));
    label = newLabel;
    sourceLabel = newSourceLabel;
    size = newSize;
    access = newAccess;
    source = nullptr;
    values.swap(newValues);
    readPosition = 0;
    writePosition = 0;
    unreadCount = 0;
    stepCount = 0;
}

void Memory::bind(Readable* newSource) {
    if (newSource == nullptr) {
        throw std::invalid_argument("Memory: source nulle");
    }
    if (newSource == this) {
        throw std::invalid_argument("Memory: une memoire ne peut pas etre sa propre source");
    }
    if (sourceLabel.empty()) {
        throw std::logic_error("Memory: charger le fichier avant bind()");
    }
    if (newSource->getLabel() != sourceLabel) {
        throw std::invalid_argument("Memory: le label de la source ne correspond pas a SOURCE: "
                                    + sourceLabel);
    }
    source = newSource;
}

void Memory::simulate() {
    if (source == nullptr) {
        throw std::logic_error("Memory '" + label + "': source non liee (bind manquant)");
    }

    ++stepCount;
    if (stepCount < access) {
        return;
    }
    stepCount = 0;

    // Lorsqu'elle est active, la memoire lit jusqu'a une valeur invalide.
    while (true) {
        const DataValue data = source->read();
        if (!data.valid) {
            break;
        }

        values[writePosition] = data.value;
        writePosition = (writePosition + 1) % values.size();

        if (unreadCount == values.size()) {
            // Memoire pleine : la nouvelle valeur remplace la plus ancienne.
            readPosition = (readPosition + 1) % values.size();
        } else {
            ++unreadCount;
        }
    }
}

DataValue Memory::read() {
    if (unreadCount == 0) {
        return DataValue{0.0, false};
    }

    const double oldest = values[readPosition];
    readPosition = (readPosition + 1) % values.size();
    --unreadCount;
    return DataValue{oldest, true};
}
