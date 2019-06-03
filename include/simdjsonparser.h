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
    
    Traverser parse(char* const json, int size) override
    {
        ParsedJson pj;
        bool allocation_is_successful = pj.allocateCapacity(size);
        if (!allocation_is_successful) {
            return Traverser(get_name(), ParsingResult::other_error);
        }
        auto simdjson_result = static_cast<simdjson::errorValues>(json_parse(json, size, pj));
        ParsingResult result = result_correspondances.at(simdjson_result);
        return Traverser(get_name(), result);
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