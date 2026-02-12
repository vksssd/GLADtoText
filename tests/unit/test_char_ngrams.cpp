#include <iostream>
#include "../../subwords/char_ngram.h"

int main() {
    Config cfg;
    cfg.char_ngram_min = 3;
    cfg.char_ngram_max = 5;

    CharNgrams extractor(cfg);

    std::vector<int> ngrams;
    extractor.extract("cancel my order too", ngrams);

    std::cout << "Extracted n-grams: " << ngrams.size() << "\n";
    for (int id : ngrams) {
        std::cout << id << " ";
    }
    std::cout << "\n";

    return 0;
}

