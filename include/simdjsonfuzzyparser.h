#ifndef SIMDJSONFUZZYPARSER_H
#define SIMDJSONFUZZYPARSER_H

#include <string>
#include <unordered_map>

#include "fuzzyjsonparser.h"
#include "simdjson.h"
#include "simdjson.cpp"

namespace fuzzyjson {

class SimdjsonFuzzyParser : public FuzzyJsonParser {
    public:
    SimdjsonFuzzyParser()
    : FuzzyJsonParser("simdjson")
    {
    };

    ~SimdjsonFuzzyParser() = default;
    
    FuzzyParserResult parse(char* const json, int size) override
    {
        ParsedJson pj;
        bool allocation_is_successful = pj.allocateCapacity(size);
        if (!allocation_is_successful) {
            return FuzzyParserResult::other_error;
        }
        simdjson::errorValues result = static_cast<simdjson::errorValues>(json_parse(json, size, pj));

        return result_correspondances.at(result);
    }

    std::string get_result_string() override {
        FuzzyParserResult fuzzy_result = result_correspondances.at(last_result);
        return result_to_string.at(fuzzy_result);
    }

    private:
    simdjson::errorValues last_result;

    std::unordered_map<simdjson::errorValues, FuzzyParserResult> result_correspondances {
        { simdjson::SUCCESS, FuzzyParserResult::ok },
        { simdjson::CAPACITY, FuzzyParserResult::other_error },
        { simdjson::TAPE_ERROR, FuzzyParserResult::other_error },
        { simdjson::DEPTH_ERROR, FuzzyParserResult::other_error },
    };
};

}

#endif