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

template<typename T>
bool floatings_are_equal(T floating1, T floating2) {
    T biggest = std::fmax(std::fabs(floating1), std::fabs(floating2));
    T diff = std::fabs(floating1 - floating2);
    return diff <= std::numeric_limits<T>::epsilon() * biggest;
}

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
    void generate_report(std::vector<std::unique_ptr<Traverser>>& traversers);
    randomjson::RandomJson random_json;
    std::vector<std::unique_ptr<Parser>> parsers;
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
    std::cout << random_json.get_generation_seed() << std::endl;
    random_json.save("test.json");
    // mutations and comparisons
    for (int no_mutation = 0; no_mutation < 10; no_mutation++) {
        std::cout << "No mutation : " << no_mutation << std::endl;
        //random_json.mutate();
        // parsing
        compare_parsing();
    }
}

void FuzzyJson::compare_parsing() {
    // getting the traversers
    std::vector<std::unique_ptr<Traverser>> traversers;
    for (auto& parser : parsers) {
        traversers.push_back(parser->parse(random_json.get_json(), random_json.get_size()));
    }

    // Compare the parsing results
    std::cout << "comparing results" << std::endl;
    ParsingResult first_result = traversers.at(0)->get_parsing_result();
    for (int i = 1; i < traversers.size(); i++) {
        // If all results are not identical, we generate a report
        if (first_result != traversers.at(i)->get_parsing_result()) {
            generate_report(traversers);
            break;
        }
    }

    // Compare parsings
    std::cout << "comparing parsings" << std::endl;
    int no_value = 0;
    const int max_values = 1024; 
    while (no_value < max_values)
    {
        ValueType current_type = traversers.at(0)->get_current_type();
        for (int i = 1; i < traversers.size(); i++) {
            // If all results are not identical, we generate a report
            if (current_type != traversers.at(i)->get_current_type()) {
                generate_report(traversers);
            }
            bool different = false;
            switch (current_type) {
                case ValueType::string : {
                    std::string leader = traversers.at(0)->get_string();
                    std::string other = traversers.at(i)->get_string();
                    different = (leader != other);
                    break;
                }
                case ValueType::key : {
                    std::string leader = traversers.at(0)->get_string();
                    std::string other = traversers.at(i)->get_string();
                    different = (leader != other);
                    break;
                }
                case ValueType::integer : {
                    int64_t leader = traversers.at(0)->get_integer();
                    int64_t other = traversers.at(i)->get_integer();
                    different = (leader != other);
                    break;
                }
                case ValueType::floating : {
                    double leader = traversers.at(0)->get_floating();
                    double other = traversers.at(i)->get_floating();
                    different = !floatings_are_equal(leader, other);
                    break;
                }
                case ValueType::boolean : {
                    double leader = traversers.at(0)->get_boolean();
                    double other = traversers.at(i)->get_boolean();
                    different = (leader != other);
                    break;
                }
                default :
                    break;
            }
            if (different) {
                generate_report(traversers);
                break;
            }
            traversers.at(i)->next();
        }
        if (current_type == ValueType::end_of_document) {
            break;
        }
        traversers.at(0)->next();
        no_value++;
    }
    
}

void FuzzyJson::generate_report(std::vector<std::unique_ptr<Traverser>>& traversers)
{
    //std::cout << "ggegeggegeg";
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
        writer.String(traverser->get_parser_name().c_str());
        writer.Key("result");
        writer.String(traverser->get_result_string().c_str());
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();
    
}

}

#endif
