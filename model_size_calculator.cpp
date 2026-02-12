#include <iostream>
#include <iomanip>
#include "core/model_config.h"

void printSizeTable(const ModelConfig& cfg) {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║          GLADtoTEXT Model Size Calculator                      ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    
    auto formatSize = [](size_t bytes) -> std::string {
        if (bytes < 1024) {
            return std::to_string(bytes) + " B";
        } else if (bytes < 1024 * 1024) {
            return std::to_string(bytes / 1024) + " KB";
        } else {
            return std::to_string(bytes / (1024 * 1024)) + " MB";
        }
    };
    
    std::cout << "Configuration:\n";
    std::cout << "  Dimension:        " << cfg.dim << "\n";
    std::cout << "  Vocabulary:       " << cfg.vocab_size << " words\n";
    std::cout << "  Bucket size:      " << cfg.bucket_size << "\n";
    std::cout << "  Classes:          " << cfg.num_classes << "\n";
    std::cout << "  Sparse matrices:  " << (cfg.use_sparse_matrices ? "ON" : "OFF") << "\n";
    std::cout << "  Quantization:     " << (cfg.use_quantization ? "ON" : "OFF") << "\n";
    std::cout << "\n";
    
    cfg.printActiveLayers();
    cfg.printSizeBreakdown();
}

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--help") {
        std::cout << "Usage: " << argv[0] << " [config_file]\n";
        std::cout << "\n";
        std::cout << "Calculate model size for different configurations.\n";
        std::cout << "\n";
        std::cout << "Examples:\n";
        std::cout << "  " << argv[0] << "                    # Use default config\n";
        std::cout << "  " << argv[0] << " model.config       # Load from file\n";
        std::cout << "\n";
        std::cout << "Predefined configs:\n";
        std::cout << "  --minimal    Minimal model (word embeddings only)\n";
        std::cout << "  --compact    Compact model (word + char n-grams)\n";
        std::cout << "  --standard   Standard model (all features except sentence)\n";
        std::cout << "  --full       Full model (all features)\n";
        std::cout << "  --tiny       Tiny model (all features, sparse)\n";
        return 0;
    }
    
    ModelConfig cfg;
    
    if (argc > 1) {
        std::string arg = argv[1];
        
        if (arg == "--minimal") {
            cfg = ModelConfigs::minimal();
            std::cout << "Using MINIMAL configuration\n";
        } else if (arg == "--compact") {
            cfg = ModelConfigs::compact();
            std::cout << "Using COMPACT configuration\n";
        } else if (arg == "--standard") {
            cfg = ModelConfigs::standard();
            std::cout << "Using STANDARD configuration\n";
        } else if (arg == "--full") {
            cfg = ModelConfigs::full();
            std::cout << "Using FULL configuration\n";
        } else if (arg == "--tiny") {
            cfg = ModelConfigs::tiny();
            std::cout << "Using TINY configuration\n";
        } else {
            // Load from file
            if (!cfg.loadFromFile(arg)) {
                std::cerr << "Error loading config from: " << arg << "\n";
                return 1;
            }
            std::cout << "Loaded configuration from: " << arg << "\n";
        }
    } else {
        cfg = ModelConfigs::standard();
        std::cout << "Using DEFAULT (standard) configuration\n";
    }
    
    printSizeTable(cfg);
    
    // Show comparison
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║          Configuration Comparison                              ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    
    auto printConfig = [&](const std::string& name, const ModelConfig& c) {
        size_t size = c.calculateTotalSize();
        std::cout << std::left << std::setw(15) << name 
                  << std::right << std::setw(10) << (size / 1024) << " KB"
                  << "  (dim=" << c.dim << ")\n";
    };
    
    printConfig("Minimal", ModelConfigs::minimal());
    printConfig("Compact", ModelConfigs::compact());
    printConfig("Standard", ModelConfigs::standard());
    printConfig("Full", ModelConfigs::full());
    printConfig("Tiny", ModelConfigs::tiny());
    
    std::cout << "\n";
    std::cout << "To generate a config file:\n";
    std::cout << "  ModelConfig cfg = ModelConfigs::tiny();\n";
    std::cout << "  cfg.saveToFile(\"model.config\");\n";
    std::cout << "\n";
    
    return 0;
}
