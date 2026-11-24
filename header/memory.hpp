#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <string>
#include <vector>
#include "Readable.hpp"

// Memoire de capacite fixe, utilisee comme tampon circulaire FIFO.
class Memory : public Readable {
private:
    std::string label;
    int size = 1;
    int access = 1;
    std::string sourceLabel;
    Readable* source = nullptr;

    std::vector<double> values;
    size_t readPosition = 0;
    size_t writePosition = 0;
    size_t unreadCount = 0;
    int stepCount = 0;

public:
    Memory() = default;

    // Phase 1 : chargement de TYPE, LABEL, SIZE, ACCESS et SOURCE.
    void loadFromFile(const std::string& filename);

    // Phase 2 : connexion a un objet Readable.
    void bind(Readable* newSource);
    void bind(Readable& newSource) { bind(&newSource); }

    // Phase 3 : lecture de toutes les valeurs disponibles tous les ACCESS cycles.
    void simulate();

    // Interface Readable. Override indique au compilateur
    // qu'une méthode redéfinit une méthode virtuelle héritée d'une classe parent.
    // Cela permet au compilateur de détecter certaines erreurs.
    DataValue read() override;
    std::string getLabel() const override { return label; }

    std::string getSourceLabel() const { return sourceLabel; }
    int getSize() const { return size; }
    int getAccess() const { return access; }
    size_t getUnreadCount() const { return unreadCount; }
    size_t getFreePlaces() const { return values.size() - unreadCount; }
};

#endif // MEMORY_HPP
