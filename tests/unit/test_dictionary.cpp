#include <iostream>
#include "../../core/dictionary.h"

int main() {
    Dictionary dict;

    dict.addWord("cancel");
    dict.addWord("order");

    std::unordered_map<std::string, int> grammar = {
        {"can", 0}, {"order", 1}
    };

    dict.finalize(1000);
    dict.loadGrammarUnits(grammar);

    int wid = dict.getWordId("cancel");
    int gid = dict.getGrammarId("order");

    std::cout << "Word ID: " << wid << "\n";
    std::cout << "Grammar ID: " << gid << "\n";

    if (wid >= 0 && gid >= 0 && gid > wid)
        std::cout << "PASS\n";
    else
        std::cout << "FAIL\n";
}
