#include "display.hpp"
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>

using namespace std;

namespace {

string trim(const string& text) {
    const string whitespace = " \t\r\n";
    const string::size_type begin = text.find_first_not_of(whitespace);
    if (begin == string::npos) {
        return "";
    }
    const string::size_type end = text.find_last_not_of(whitespace);
    return text.substr(begin, end - begin + 1);
}

string upper(string text) {
    for (char& c : text) {
        c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    }
    return text;
}

string requiredField(const map<string, string>& fields,
                          const string& key,
                          const string& filename) {
    const auto it = fields.find(key);
    if (it == fields.end() || it->second.empty()) {
        throw runtime_error("Display: champ " + key + " manquant dans " + filename);
    }
    return it->second;
}

} // namespace

void Display::loadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file) {
        throw runtime_error("Display: impossible d'ouvrir " + filename);
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
            throw runtime_error("Display: ligne invalide " + to_string(lineNumber)
                                     + " dans " + filename);
        }

        const string key = upper(trim(line.substr(0, separator)));
        const string value = trim(line.substr(separator + 1));
        if (key != "TYPE" && key != "REFRESH" && key != "SOURCE") {
            throw runtime_error("Display: champ inconnu '" + key + "' dans " + filename);
        }
        if (fields.count(key) != 0) {
            throw runtime_error("Display: champ " + key + " en double dans " + filename);
        }
        fields.emplace(key, value);
    }

    if (upper(requiredField(fields, "TYPE", filename)) != "DISPLAY") {
        throw runtime_error("Display: TYPE doit etre DISPLAY dans " + filename);
    }

    const string newSourceLabel = requiredField(fields, "SOURCE", filename);
    const string refreshText = requiredField(fields, "REFRESH", filename);
    size_t parsedLength = 0;
    int newRefresh = 0;
    try {
        newRefresh = stoi(refreshText, &parsedLength);
    } catch (const exception&) {
        throw runtime_error("Display: REFRESH invalide dans " + filename);
    }
    if (parsedLength != refreshText.size() || newRefresh <= 0) {
        throw runtime_error("Display: REFRESH doit etre un entier strictement positif dans " + filename);
    }

    sourceLabel = newSourceLabel;
    refresh = newRefresh;
    source = nullptr;
    stepCount = 0;
}

void Display::bind(Readable* newSource) {
    if (newSource == nullptr) {
        throw invalid_argument("Display: source nulle");
    }
    if (sourceLabel.empty()) {
        throw logic_error("Display: charger le fichier avant bind()");
    }
    source = newSource;
}

void Display::simulate() {
    if (source == nullptr) {
        throw logic_error("Display: source non liee (bind manquant)");
    }

    ++stepCount;
    if (stepCount < refresh) {
        return;
    }
    stepCount = 0;

    while (true) {
        const DataValue data = source->read();
        if (!data.valid) {
            break;
        }
        cout << data.value << endl;
    }
}
