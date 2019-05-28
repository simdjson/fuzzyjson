#ifndef MOCKFUZZYPARSER_H
#define MOCKFUZZYPARSER_H

#include <string>

#include "fuzzyjsonparser.h"

namespace fuzzyjson
{

class MockFuzzyParser : public FuzzyJsonParser
{
    public:
    MockFuzzyParser(std::string name, FuzzyParserResult result)
    : FuzzyJsonParser(name)
    , last_result(result)
    {
    };

    ~MockFuzzyParser() = default;
    
    FuzzyParserResult parse(char* const json, int size) override
    {
        return last_result;
    }

    std::string get_result_string() override {
        return result_to_string.at(last_result);
    }

    private:
    FuzzyParserResult last_result;
};

}

#endif