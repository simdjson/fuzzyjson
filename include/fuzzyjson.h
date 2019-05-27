#ifndef FUZZYJSON_H
#define FUZZYJSON_H

#include <cstring>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <memory>
#include <random>
#include <vector>

#include "randomjson.h"
#include "fuzzyjsonparser.h"

namespace fuzzyjson {
class FuzzyJson {
    public:
    FuzzyJson(int size); // a random json file will be generated
    FuzzyJson(std::string json_filename);
    void add_parser(std::unique_ptr<fuzzyjson::FuzzyJsonParser>&& parser) {
        parsers.push_back(std::move(parser));
    }
    void fuzz();

    private:
    void generate_report();
    randomjson::RandomJson random_json;
    std::string source_type;
    std::string source_name;
    int seed;
    std::vector<std::unique_ptr<fuzzyjson::FuzzyJsonParser>> parsers;
    std::random_device rd;
    std::mt19937 random_generator;
};

FuzzyJson::FuzzyJson(int size)
: random_json(size)
, source_type("randomjson")
, source_name(std::to_string(random_json.get_seed()))
, seed(rd())
, random_generator(seed)
{
}

FuzzyJson::FuzzyJson(std::string json_filename)
: random_json(json_filename)
, source_type("file")
, source_name(json_filename)
, seed(rd())
, random_generator(seed)
{
}

void FuzzyJson::fuzz()
{
    // initializing the results
    std::vector<FuzzyParserResult> results(parsers.size());
    for (auto& parser : parsers) {
        results.push_back(FuzzyParserResult::ok);
    }

    for (int iteration_number = 0; iteration_number < 10; iteration_number++) {
        random_json.mutate();
        // parsing
        for (int parser_index = 0; parser_index < parsers.size(); parser_index++) {
            auto& parser = parsers[parser_index];
            results.at(parser_index) = parser->parse(random_json.get_json(), random_json.get_size());
        }
        // comparing the results
        // To fix : will crash if there is less than two parser.
        FuzzyParserResult first_result = results.at(0);
        for (int parser_index = 1; parser_index < results.size(); parser_index++) {
            // If all results are not identical, we generate a report
            if (first_result != results.at(parser_index)) {
                generate_report();
            }
        }
    }
}

void FuzzyJson::generate_report() {
    
}

}

#endif
