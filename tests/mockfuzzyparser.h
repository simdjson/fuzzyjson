#ifndef MOCKFUZZYPARSER_H
#define MOCKFUZZYPARSER_H

#include <string>
#include <unordered_map>

#include "fuzzyjsonparser.h"

namespace fuzzyjson {

class MockFuzzyParser : public FuzzyJsonParser {
    public:
    MockFuzzyParser(std::string name, FuzzyParserResult result)
    : FuzzyJsonParser(name)
    , current_result(result)
    {
    };
    
    FuzzyParserResult parse(char* json, int size) override
    {
        return current_result;
    }

    private:
    FuzzyParserResult current_result;
};

}

#endif