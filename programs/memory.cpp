#include "memory.hpp"
#include "ConfigFile.hpp"
#include <stdexcept>

void Memory::loadFromFile(const std::string& filename) {
    ConfigFile config(filename);
    if (config.get("TYPE") != "MEMORY") {
        throw std::runtime_error("Memory: TYPE doit etre MEMORY dans " + filename);
    }

    const std::string newLabel = config.get("LABEL");
    const std::string newSourceLabel = config.get("SOURCE");
    const int newSize = config.getInt("SIZE", 1);
    const int newAccess = config.getInt("ACCESS", 1);
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

