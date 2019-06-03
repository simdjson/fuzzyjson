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

#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "randomjson.h"
#include "parser.h"


namespace fuzzyjson
{

class FuzzyJson
{
    public:
    FuzzyJson(int size); // a random json file will be generated
    FuzzyJson(std::string json_filename);
    void add_parser(std::unique_ptr<Parser> parser) {
        parsers.push_back(std::move(parser));
    }
    void fuzz();

    private:
    void compare_parsing();
    void generate_report(std::vector<Traverser> traversers);
    randomjson::RandomJson random_json;
    std::vector<std::unique_ptr< Parser>> parsers;
};

FuzzyJson::FuzzyJson(int size)
: random_json(size)
{
}

FuzzyJson::FuzzyJson(std::string json_filename)
: random_json(json_filename)
{
}

void FuzzyJson::fuzz()
{
    // mutations and comparisons
    for (int iteration_number = 0; iteration_number < 10; iteration_number++) {
        //random_json.mutate();
        // parsing
        compare_parsing();
    }
}

void FuzzyJson::compare_parsing() {
    std::vector<Traverser> traversers;

    for (auto& parser : parsers) {
        traversers.push_back(parser->parse(random_json.get_json(), random_json.get_size()));
    }
    // comparing the results
    // To fix : will crash if there is less than two parser.
    ParsingResult first_result = traversers.at(0).get_parsing_result();
    for (int parser_index = 1; parser_index < traversers.size(); parser_index++) {
        // If all results are not identical, we generate a report
        if (first_result != traversers.at(parser_index).get_parsing_result()) {
            generate_report(traversers);
        }
    }
}

void FuzzyJson::generate_report(std::vector<Traverser> traversers)
{
    rapidjson::StringBuffer string_buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(string_buffer);
    writer.StartObject();
    writer.Key("randomjson");
    writer.StartObject();
    writer.Key("provenance_type");
    switch (random_json.get_provenance_type()) {
        case randomjson::ProvenanceType::seed :
            writer.String("seed");
            writer.Key("source_name");
            writer.Uint(random_json.get_generation_seed());
            break;
        case randomjson::ProvenanceType::file :
            writer.String("file");
            writer.Key("filename");
            writer.String(random_json.get_filename().c_str());
            break;
    }
    writer.Key("mutation_seed");
    writer.Uint(random_json.get_mutation_seed());
    writer.Key("number_of_mutations");
    writer.Uint(random_json.get_number_of_mutations());
    writer.EndObject();
    writer.Key("parsing_results");
    writer.StartArray();
    for (auto& traverser : traversers) {
        writer.StartObject();
        writer.Key("parser_name");
        writer.String(traverser.get_parser_name().c_str());
        writer.Key("result");
        writer.String(traverser.get_result_string().c_str());
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();
    
}

}

#endif
