#include "platform.hpp"
#include <cctype>
#include <fstream>
#include <map>
#include <stdexcept>
#include <utility>

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

// Essaie d'abord le chemin du fichier, puis celui relatif a platform.txt.
std::string componentPath(const std::string& path, const std::string& platformFile) {
    std::ifstream direct(path);
    if (direct) {
        return path;
    }

    const std::string::size_type slash = platformFile.find_last_of("/\\");
    if (slash != std::string::npos) {
        const std::string relative = platformFile.substr(0, slash + 1) + path;
        std::ifstream candidate(relative);
        if (candidate) {
            return relative;
        }
    }
    throw std::runtime_error("Platform: composant introuvable : " + path);
}

std::string componentType(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Platform: impossible d'ouvrir " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const std::string::size_type separator = line.find(':');
        if (separator == std::string::npos) {
            throw std::runtime_error("Platform: ligne invalide dans " + filename);
        }
        if (upper(trim(line.substr(0, separator))) == "TYPE") {
            return upper(trim(line.substr(separator + 1)));
        }
    }
    throw std::runtime_error("Platform: TYPE manquant dans " + filename);
}

} // namespace

void Platform::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Platform: impossible d'ouvrir " + filename);
    }

    // On charge dans des conteneurs temporaires : en cas d'erreur,
    // la plateforme actuelle reste inchangee.
    std::vector<std::unique_ptr<CPU>> newCpus;
    std::vector<std::unique_ptr<Bus>> newBuses;
    std::vector<std::unique_ptr<Memory>> newMemories;
    std::vector<std::unique_ptr<Display>> newDisplays;
    std::string line;
    while (std::getline(file, line)) {
        const std::string entry = trim(line);
        if (entry.empty() || entry[0] == '#') {
            continue;
        }

        const std::string path = componentPath(entry, filename);
        const std::string type = componentType(path);

        if (type == "CPU") {
            std::unique_ptr<CPU> cpu(new CPU());
            cpu->loadFromFile(path);
            newCpus.push_back(std::move(cpu));
        } else if (type == "BUS") {
            std::unique_ptr<Bus> bus(new Bus());
            bus->loadFromFile(path);
            newBuses.push_back(std::move(bus));
        } else if (type == "MEMORY") {
            std::unique_ptr<Memory> memory(new Memory());
            memory->loadFromFile(path);
            newMemories.push_back(std::move(memory));
        } else if (type == "DISPLAY") {
            std::unique_ptr<Display> display(new Display());
            display->loadFromFile(path);
            newDisplays.push_back(std::move(display));
        } else {
            throw std::runtime_error("Platform: TYPE inconnu '" + type + "' dans " + path);
        }
    }

    cpus.swap(newCpus);
    buses.swap(newBuses);
    memories.swap(newMemories);
    displays.swap(newDisplays);
    bound = false;
}

void Platform::bind() {
    // Un label identifie une source lisible de maniere unique.
    std::map<std::string, Readable*> sources;
    for (const auto& cpu : cpus) {
        if (!sources.emplace(cpu->getLabel(), cpu.get()).second) {
            throw std::runtime_error("Platform: label duplique : " + cpu->getLabel());
        }
    }
    for (const auto& bus : buses) {
        if (!sources.emplace(bus->getLabel(), bus.get()).second) {
            throw std::runtime_error("Platform: label duplique : " + bus->getLabel());
        }
    }
    for (const auto& memory : memories) {
        if (!sources.emplace(memory->getLabel(), memory.get()).second) {
            throw std::runtime_error("Platform: label duplique : " + memory->getLabel());
        }
    }

    // Resoudre toutes les sources avant d'effectuer les connexions.
    std::vector<Readable*> busSources;
    std::vector<Readable*> memorySources;
    std::vector<Readable*> displaySources;
    for (const auto& bus : buses) {
        const auto it = sources.find(bus->getSourceLabel());
        if (it == sources.end()) {
            throw std::runtime_error("Platform: source introuvable : " + bus->getSourceLabel());
        }
        busSources.push_back(it->second);
    }
    for (const auto& memory : memories) {
        const auto it = sources.find(memory->getSourceLabel());
        if (it == sources.end()) {
            throw std::runtime_error("Platform: source introuvable : " + memory->getSourceLabel());
        }
        memorySources.push_back(it->second);
    }
    for (const auto& display : displays) {
        const auto it = sources.find(display->getSourceLabel());
        if (it == sources.end()) {
            throw std::runtime_error("Platform: source introuvable : " + display->getSourceLabel());
        }
        displaySources.push_back(it->second);
    }

    bound = false;
    for (size_t i = 0; i < buses.size(); ++i) {
        buses[i]->bind(busSources[i]);
    }
    for (size_t i = 0; i < memories.size(); ++i) {
        memories[i]->bind(memorySources[i]);
    }
    for (size_t i = 0; i < displays.size(); ++i) {
        displays[i]->bind(displaySources[i]);
    }
    bound = true;
}

void Platform::simulate() {
    if (!bound) {
        throw std::logic_error("Platform: appeler bind() avant simulate()");
    }

    // Ordre fixe pour rendre le resultat reproductible.
    for (const auto& cpu : cpus) {
        cpu->simulate();
    }
    for (const auto& bus : buses) {
        bus->simulate();
    }
    for (const auto& memory : memories) {
        memory->simulate();
    }
    for (const auto& display : displays) {
        display->simulate();
    }
}

void Platform::simulate(int steps) {
    if (steps < 0) {
        throw std::invalid_argument("Platform: nombre de cycles negatif");
    }
    for (int i = 0; i < steps; ++i) {
        simulate();
    }
}
