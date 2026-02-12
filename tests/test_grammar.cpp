#include <iostream>
#include "../subwords/grammar_units.h"
#include "../subwords/phonetic.h"

int main(){
    GrammarUnitInducer inducer(3,6,2);

    inducer.observe("cancel");
    inducer.observe("cancel");
    inducer.observe("cancel");
    
    std::unordered_map<std::string, int>vocab;
    inducer.finalize(vocab);

    std::cout<<"Grammer Units: \n";
    for(auto& kv: vocab){
        std::cout<<kv.first<<"->"<<kv.second<<"\n";
    }

    std::cout<<"Phonetic cancel: "<<phoneticEncode("cancel") << "\n";
    std::cout<<"Phonetic cancel: "<<phoneticEncode("cancel") << "\n";
    

}