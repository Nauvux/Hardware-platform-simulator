#include "platform.hpp"
#include "ConfigFile.hpp"
#include "Bindable.hpp"
#include <fstream>
#include <map>
#include <stdexcept>
#include <utility>

namespace {

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
    ConfigFile config(filename);
    return config.get("TYPE");
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

