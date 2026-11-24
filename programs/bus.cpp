#include "bus.hpp"
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <map>

using namespace std;

namespace {

//enleve les espaces en debut et fin de chaine
string trim(const string& text) {
    const string whitespace = " \t\r\n";
    const string::size_type begin = text.find_first_not_of(whitespace);
    if (begin == string::npos) {
        return "";
    }
    const string::size_type end = text.find_last_not_of(whitespace);
    return text.substr(begin, end - begin + 1);
}

// Converts a string to uppercase
string upper(string text) {
    for (char& c : text) {
        c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    }
    return text;
}

// verifie que le champ key est present dans fields et retourne sa valeur.
string requiredField(const map<string, string>& fields,
                          const string& key,
                          const string& filename) {
    const auto it = fields.find(key);
    if (it == fields.end() || it->second.empty()) {
        throw runtime_error("Bus: champ " + key + " manquant dans " + filename);
    }
    return it->second;
}

} // namespace

// Charge le bus depuis un fichier texte.
// Le fichier doit contenir les champs TYPE, LABEL, WIDTH et SOURCE.

void Bus::loadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file) {
        throw runtime_error("Bus: impossible d'ouvrir " + filename);
    }

    map<string, string> fields;
    string line;
    size_t lineNumber = 0;

    while (getline(file, line)) {
        ++lineNumber;
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        const string::size_type separator = line.find(':');
        if (separator == string::npos) {
            throw runtime_error("Bus: ligne invalide " + to_string(lineNumber)
                                     + " dans " + filename);
        }

        const string key = upper(trim(line.substr(0, separator)));
        const string value = trim(line.substr(separator + 1));
        if (key != "TYPE" && key != "LABEL" && key != "WIDTH" && key != "SOURCE") {
            throw runtime_error("Bus: champ inconnu '" + key + "' dans " + filename);
        }
        if (fields.count(key) != 0) {
            throw runtime_error("Bus: champ " + key + " en double dans " + filename);
        }
        fields.emplace(key, value);
    }

    if (upper(requiredField(fields, "TYPE", filename)) != "BUS") {
        throw runtime_error("Bus: TYPE doit etre BUS dans " + filename);
    }

    const string newLabel = requiredField(fields, "LABEL", filename);
    const string newSourceLabel = requiredField(fields, "SOURCE", filename);
    const string widthText = requiredField(fields, "WIDTH", filename);

    size_t parsedLength = 0;
    int newWidth = 0;
    try {
        newWidth = stoi(widthText, &parsedLength);
    } catch (const exception&) {
        throw runtime_error("Bus: WIDTH invalide dans " + filename);
    }
    if (parsedLength != widthText.size() || newWidth <= 0) {
        throw runtime_error("Bus: WIDTH doit etre un entier strictement positif dans " + filename);
    }

    // On ne modifie l'etat que lorsque le fichier entier est valide.
    label = newLabel;
    sourceLabel = newSourceLabel;
    width = newWidth;
    source = nullptr; // Un nouveau chargement necessite un nouveau bind().
    pending = queue<DataValue>();
    ready = queue<DataValue>();
    readCount = 0;
}

// Connecte le bus a sa source.
void Bus::bind(Readable* newSource) {
    if (newSource == nullptr) {
        throw invalid_argument("Bus: source nulle");
    }
    if (newSource == this) {
        throw invalid_argument("Bus: un bus ne peut pas etre sa propre source");
    }
    if (sourceLabel.empty()) {
        throw logic_error("Bus: charger le fichier avant bind()");
    }
    if (newSource->getLabel() != sourceLabel) {
        throw invalid_argument("Bus: le label de la source ne correspond pas a SOURCE: "
                                    + sourceLabel);
    }
    source = newSource;
}

// Transfert les donees avec un cycle de latence
void Bus::simulate() {
    if (source == nullptr) {
        throw logic_error("Bus '" + label + "': source non liee (bind manquant)");
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