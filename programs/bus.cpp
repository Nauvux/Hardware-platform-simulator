#include "bus.hpp"
#include "ConfigFile.hpp"
#include <stdexcept>

void Bus::loadFromFile(const std::string& filename) {
    ConfigFile config(filename);
    if (config.get("TYPE") != "BUS") {
        throw std::runtime_error("Bus: TYPE doit etre BUS dans " + filename);
    }

    const std::string newLabel = config.get("LABEL");
    const std::string newSourceLabel = config.get("SOURCE");
    const int newWidth = config.getInt("WIDTH", 1);

    label = newLabel;
    sourceLabel = newSourceLabel;
    width = newWidth;
    source = nullptr;
    pending = std::queue<DataValue>();
    ready = std::queue<DataValue>();
    readCount = 0;
}

void Bus::bind(Readable* newSource) {
    if (newSource == nullptr) {
        throw std::invalid_argument("Bus: source nulle");
    }
    if (newSource == this) {
        throw std::invalid_argument("Bus: un bus ne peut pas etre sa propre source");
    }
    if (sourceLabel.empty()) {
        throw std::logic_error("Bus: charger le fichier avant bind()");
    }
    if (newSource->getLabel() != sourceLabel) {
        throw std::invalid_argument("Bus: le label de la source ne correspond pas a SOURCE: "
                                    + sourceLabel);
    }
    source = newSource;
}

void Bus::simulate() {
    if (source == nullptr) {
        throw std::logic_error("Bus '" + label + "': source non liee (bind manquant)");
    }

    // Les donnees lues au tour precedent deviennent disponibles.
    while (!pending.empty()) {
        ready.push(pending.front());
        pending.pop();
    }

    // Les nouvelles donnees arriveront seulement au tour suivant.
    for (int i = 0; i < width; ++i) {
        const DataValue data = source->read();
        if (!data.valid) {
            break;
        }
        pending.push(data);
    }
}

DataValue Bus::read() {
    ++readCount; // Compte aussi les tentatives de lecture infructueuses.
    if (ready.empty()) {
        return DataValue{0.0, false};
    }

    const DataValue oldest = ready.front();
    ready.pop();
    return oldest;
}

