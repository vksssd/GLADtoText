#include <iostream>
#include <unordered_map>
#include "../subwords/grammar_units.h"
#include "../subwords/phonetic.h"

int main() {
    // MinLen 3, MaxLen 6, MinFreq 2
    GrammarUnitInducer inducer(3, 6, 2);

    // Grouping different spellings that sound the same
    std::vector<std::string> words = {"cancel", "kancel", "candsel", "cancel"};

    std::cout << "--- Phonetic Encoding Phase ---\n";
    for (const auto& w : words) {
        std::string encoded = phoneticEncode(w);
        std::cout << w << " encodes to: " << encoded << "\n";
        inducer.observe(encoded); // Inducing on the SOUND, not the spelling
    }

    std::unordered_map<std::string, int> vocab;
    inducer.finalize(vocab);

    std::cout << "\n--- Induced Phonetic Grammar Units ---\n";
    if (vocab.empty()) {
        std::cout << "No units met the frequency threshold.\n";
    } else {
        for (auto const& [unit, id] : vocab) {
            std::cout << "Phonetic Unit [" << unit << "] -> ID: " << id << "\n";
        }
    }

    return 0;
}