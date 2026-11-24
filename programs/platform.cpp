#include "platform.hpp"
#include "CPU.hpp"
#include "bus.hpp"
#include "memory.hpp"
#include "display.hpp"
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

    // On charge dans un conteneur temporaire : en cas d'erreur,
    // la plateforme actuelle reste inchangee.
    std::vector<std::unique_ptr<Component>> newComponents;
    std::string line;
    while (std::getline(file, line)) {
        const std::string entry = trim(line);
        if (entry.empty() || entry[0] == '#') {
            continue;
        }

        const std::string path = componentPath(entry, filename);
        const std::string type = componentType(path);

        // Seul endroit ou l'on a besoin du type exact : la creation.
        if (type == "CPU") {
            std::unique_ptr<CPU> cpu(new CPU());
            cpu->loadFromFile(path);
            newComponents.push_back(std::move(cpu));
        } else if (type == "BUS") {
            std::unique_ptr<Bus> bus(new Bus());
            bus->loadFromFile(path);
            newComponents.push_back(std::move(bus));
        } else if (type == "MEMORY") {
            std::unique_ptr<Memory> memory(new Memory());
            memory->loadFromFile(path);
            newComponents.push_back(std::move(memory));
        } else if (type == "DISPLAY") {
            std::unique_ptr<Display> display(new Display());
            display->loadFromFile(path);
            newComponents.push_back(std::move(display));
        } else {
            throw std::runtime_error("Platform: TYPE inconnu '" + type + "' dans " + path);
        }
    }

    components.swap(newComponents);
    bound = false;
}

void Platform::bind() {
    // Un label identifie une source lisible de maniere unique.
    // dynamic_cast renvoie nullptr si le composant n'est pas Readable (ex. Display).
    std::map<std::string, Readable*> sources;
    for (const auto& component : components) {
        Readable* readable = dynamic_cast<Readable*>(component.get());
        if (readable == nullptr) {
            continue;
        }
        if (!sources.emplace(component->getLabel(), readable).second) {
            throw std::runtime_error("Platform: label duplique : " + component->getLabel());
        }
    }

    // Resoudre toutes les sources avant d'effectuer les connexions.
    std::vector<std::pair<Bindable*, Readable*>> links;
    for (const auto& component : components) {
        Bindable* bindable = dynamic_cast<Bindable*>(component.get());
        if (bindable == nullptr) {
            continue; // ex. un CPU n'a pas de source
        }
        const auto it = sources.find(bindable->getSourceLabel());
        if (it == sources.end()) {
            throw std::runtime_error("Platform: source introuvable : " + bindable->getSourceLabel());
        }
        links.emplace_back(bindable, it->second);
    }

    bound = false;
    for (const auto& link : links) {
        link.first->bind(link.second);
    }
    bound = true;
}

void Platform::simulate() {
    if (!bound) {
        throw std::logic_error("Platform: appeler bind() avant simulate()");
    }

    // Polymorphisme : chaque composant execute SA version de simulate().
    // Ordre = ordre du fichier platform.txt.
    for (const auto& component : components) {
        component->simulate();
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
