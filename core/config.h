#pragma once

struct Config {
    int dim = 100;

    int char_ngram_min = 3;
    int char_ngram_max = 6;

    int bucket_size = 2000000; // fastText-style hashing
};
