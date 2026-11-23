#ifndef BUS_HPP
#define BUS_HPP

#include <queue>
#include <string>
#include "Readable.hpp"

// Un bus lit une source Readable et transmet ses donnees avec
// un cycle de simulation de latence.
class Bus : public Readable {
private:
    std::string label;
    int width = 1;
    std::string sourceLabel;

    // Pointeur non proprietaire : la Platform gere la duree de vie des composants.
    Readable* source = nullptr;

    std::queue<DataValue> pending;
    std::queue<DataValue> ready;
    size_t readCount = 0;

public:
    Bus() = default;

    // Phase 1 : charge TYPE, LABEL, WIDTH et SOURCE depuis un fichier texte.
    void loadFromFile(const std::string& filename);

    // Phase 2 : associe au bus la source dont le label figure dans le fichier.
    void bind(Readable* newSource);
    void bind(Readable& newSource) { bind(&newSource); }

    // Phase 3 : rend disponibles les anciennes donnees pending, puis lit
    // au maximum WIDTH nouvelles donnees, qui restent pending jusqu'au
    // prochain appel de simulate().
    void simulate();

    // Interface Readable : lit la plus ancienne donnee prete (FIFO).
    DataValue read() override;
    std::string getLabel() const override { return label; }

    std::string getSourceLabel() const { return sourceLabel; }
    int getWidth() const { return width; }
    size_t getReadCount() const { return readCount; }
};

#endif // BUS_HPP

