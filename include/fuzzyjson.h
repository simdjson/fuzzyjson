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

// declarations
namespace fuzzyjson {
class FuzzyJson {
    public:
    FuzzyJson(int size);
    FuzzyJson(std::string json_filename);
    void add_parser(std::unique_ptr<fuzzyjson::FuzzyJsonParser>&& parser) {
        parsers.push_back(std::move(parser));
    }
    void fuzz();

    private:
    void generate_report();
    randomjson::RandomJson random_json;
    std::string name;
    int seed;
    int iteration_number;
    std::vector<std::unique_ptr<fuzzyjson::FuzzyJsonParser>> parsers;
    std::random_device rd;
    std::mt19937 random_generator;
};


FuzzyJson::FuzzyJson(int size)
: random_json(size)
, name("randomjson: " + std::to_string(random_json.get_seed()))
, seed(rd())
, iteration_number(0)
, random_generator(seed)
{
}

FuzzyJson::FuzzyJson(std::string json_filename)
: random_json(json_filename)
, name("file:"+json_filename)
, seed(rd())
, iteration_number(0)
, random_generator(seed)
{
}

void FuzzyJson::fuzz()
{
    for (; iteration_number < 10; iteration_number++){
        std::cout << "num: " << iteration_number << std::endl;
        random_json.mutate();
        for (auto& parser : parsers) {
            parser->parse(random_json.get_json(), random_json.get_size());
        }
    }
}
}

#endif
