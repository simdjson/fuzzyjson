#ifndef SIMDJSONPARSER_H
#define SIMDJSONPARSER_H

#include <string>
#include <unordered_map>

#include "parser.h"
#include "simdjson.h"
#include "simdjson.cpp"

namespace fuzzyjson {

class SimdjsonParser : public Parser {
    public:
    SimdjsonParser()
    : Parser("simdjson")
    {
    };

    ~SimdjsonParser() = default;
    
    ParsingResult parse(char* const json, int size) override
    {
        ParsedJson pj;
        bool allocation_is_successful = pj.allocateCapacity(size);
        if (!allocation_is_successful) {
            return ParsingResult::other_error;
        }
        simdjson::errorValues result = static_cast<simdjson::errorValues>(json_parse(json, size, pj));

        return result_correspondances.at(result);
    }

    std::string get_result_string() override {
        ParsingResult result = result_correspondances.at(last_result);
        return result_to_string.at(result);
    }

    private:
    simdjson::errorValues last_result;

    std::unordered_map<simdjson::errorValues, ParsingResult> result_correspondances {
        { simdjson::SUCCESS, ParsingResult::ok },
        { simdjson::CAPACITY, ParsingResult::other_error },
        { simdjson::TAPE_ERROR, ParsingResult::other_error },
        { simdjson::DEPTH_ERROR, ParsingResult::other_error },
    };
};

}

#endif