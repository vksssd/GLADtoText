#pragma once

#include <string>
#include <vector>

class ITokenizer {
public:
    virtual ~ITokenizer() = default;
    
    virtual void tokenize(const std::string& text,
                         std::vector<std::string>& tokens) const = 0;
};
