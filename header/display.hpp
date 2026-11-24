#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <string>
#include "Readable.hpp"
#include "Bindable.hpp"

// Un afficheur lit sa source tous les REFRESH cycles.
class Display : public Bindable {
private:
    int refresh = 1;
    int stepCount = 0;
    std::string sourceLabel;
    Readable* source = nullptr;

public:
    Display() = default;

    // Phase 1 : charge TYPE, REFRESH et SOURCE.
    void loadFromFile(const std::string& filename);

    // Phase 2 : connexion a un composant lisible.
    void bind(Readable* newSource) override;
    void bind(Readable& newSource) { bind(&newSource); }

    // Phase 3 : affiche toutes les valeurs disponibles au rythme REFRESH.
    void simulate();

    std::string getSourceLabel() const override { return sourceLabel; }
    int getRefresh() const { return refresh; }
};

#endif // DISPLAY_HPP

