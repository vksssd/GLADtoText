#include "../heads/semantic.h"
#include "../subwords/char_ngrams.h"
#include "../core/dictionary.h"
#include <iostream>

void trainUnsupervised(
    Backbone& backbone,
    Embeddings& emb,
    VectorAttention& att,
    Dictionary& dict,
    const CharNgrams& ngrams,
    const std::vector<std::vector<int>>& corpus,
    int epochs
) {
    int vocab = dict.totalSize();
    SemanticTrainer trainer(vocab, backbone.dim, &backbone, 0.05f, 5);
    NegativeSampler sampler(vocab);

    for (int ep = 0; ep < epochs; ep++) {
        for (const auto& sent : corpus) {

            for (size_t j = 0; j < sent.size(); j++) {
                int center = sent[j];

                // Build subword ids for center
                auto center_ids = getSubwordIds(center, dict, ngrams);

                // Context window = 1
                std::vector<int> context_word_ids;
                if (j > 0) context_word_ids.push_back(sent[j - 1]);
                if (j + 1 < sent.size()) context_word_ids.push_back(sent[j + 1]);

                if (context_word_ids.empty()) continue;

                // Build subword ids for context
                std::vector<int> context_subword_ids;
                for (int w : context_word_ids) {
                    auto ids = getSubwordIds(w, dict, ngrams);
                    context_subword_ids.insert(
                        context_subword_ids.end(),
                        ids.begin(), ids.end());
                }

                // CBOW
                trainer.cbowUpdate(context_subword_ids, center, sampler);

                // Skipgram
                trainer.skipgramUpdate(center_ids,
                                       context_word_ids,
                                       sampler);
            }
        }

        std::cout << "Epoch " << ep << " done\n";
    }
}
