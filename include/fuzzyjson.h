#ifndef FUZZYJSON_H
#define FUZZYJSON_H

#include <cstdlib>
#include <cstring>
#include <chrono>
#include <ctime>
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

bool floatings_are_equal(double floating1, double floating2, double precision) {
    double biggest = std::fmax(std::fabs(floating1), std::fabs(floating2));
    double diff = std::fabs(floating1 - floating2);
    return diff <= precision*biggest;
}

struct FuzzyJsonSettings {
    int id = 0; // An identifier for the processus. Used in parallelism.
    int max_mutations = 10000;
    double comparison_precision = std::numeric_limits<double>::epsilon();
    bool verbose = false;
};

class FuzzyJson
{
    public:
    FuzzyJson(const randomjson::RandomJsonSettings& json_settings, FuzzyJsonSettings fuzzy_settings = FuzzyJsonSettings{});

    void add_parser(std::unique_ptr<Parser> parser) {
        parsers.push_back(std::move(parser));
    }
    void fuzz();

    private:
    void compare_parsing();
    void generate_report(std::vector<std::unique_ptr<Traverser>>& traversers, int value_index);
    void reverse_mutation();
    std::string id;
    randomjson::RandomJson random_json;
    FuzzyJsonSettings settings;
    std::vector<std::unique_ptr<Parser>> parsers;
};

FuzzyJson::FuzzyJson(const randomjson::RandomJsonSettings& json_settings, FuzzyJsonSettings fuzzy_settings)
: id(std::to_string(fuzzy_settings.id))
, random_json(json_settings)
, settings(fuzzy_settings)
{
    std::cout << id << std::endl;
}

void FuzzyJson::fuzz()
{
    if (settings.verbose) {
        std::cout << "seed " << random_json.get_generation_seed() << std::endl;
    }
    std::string copy_name = std::string("temp-")+id+".json";
    // mutations and comparisons
    for (int no_mutation = 0; no_mutation < settings.max_mutations; no_mutation++) {
        // we save a copy in case of unexpected crash
        // That's not very intelligent.
        random_json.save(copy_name);
        compare_parsing();
        if (settings.verbose) {
            std::cout << "mutation " << no_mutation << std::endl;
        }
        random_json.mutate();
    }

    std::remove(copy_name.c_str());
}

void FuzzyJson::reverse_mutation() {
    if (settings.verbose) {
        std::cout << "reverse " << random_json.get_number_of_mutations() << std::endl;
    }
    random_json.reverse_mutation();
}

void FuzzyJson::compare_parsing()
{
    // getting the traversers
    // The first traverser will be compared to all the others.
    std::vector<std::unique_ptr<Traverser>> traversers;
    for (auto& parser : parsers) {
        traversers.push_back(parser->parse(random_json.get_json(), random_json.get_size()));
    }

    ParsingState first_parsing_state = traversers.at(0)->get_parsing_state();

    // Compare the parsing states
    for (int i = 1; i < traversers.size(); i++) {
        if (first_parsing_state != traversers.at(i)->get_parsing_state()) {
            generate_report(traversers, -1);
            // if there was no mutation, then there is probably a problem with the json generator.
            if (random_json.get_number_of_mutations() == 0) {
                // anything else more intelligent to do ?
                std::abort();
            }
            reverse_mutation();
            return;
        }
    }

    // If we arrive here and the first parsing failed, then all parsings failed
    // We just reverse the last mutation but we're not interested by the report
    if (first_parsing_state == ParsingState::error) {
        // constestable copypaste
        // if there was no mutation, then there is probably a problem with the json generator.
        if (random_json.get_number_of_mutations() == 0) {
            generate_report(traversers, -1); // the difference from the copypaste is here
            // anything else more intelligent to do ?
            std::abort();
        }
        reverse_mutation();
        return;
    }    

    // Compare parsings
    int value_index = 0;
    const int max_values = 1024; 
    while (value_index < max_values)
    {
        ValueType current_type = traversers.at(0)->get_current_type();
        for (int i = 1; i < traversers.size(); i++) {
            // If all types are not identical, we generate a report
            if (current_type != traversers.at(i)->get_current_type()) {
                generate_report(traversers, value_index);
                traversers.at(i)->next();
                continue; // the program could crash if we try to read a wrong type
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
                    //different = !floatings_are_equal(leader, other);
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
                generate_report(traversers, value_index);
            }
            traversers.at(i)->next();
        }
        if (current_type == ValueType::end_of_document) {
            break;
        }
        traversers.at(0)->next();
        value_index++;
    }
}

void FuzzyJson::generate_report(std::vector<std::unique_ptr<Traverser>>& traversers, int value_index)
{
    if (settings.verbose) {
        std::cout << "report" << std::endl;
    }
    auto nanoseconds = std::chrono::system_clock::now();
    std::string time = std::to_string(std::chrono::system_clock::to_time_t(nanoseconds));
    rapidjson::StringBuffer string_buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(string_buffer);
    writer.StartObject();
    writer.Key("randomjson");
    writer.StartObject();
    writer.Key("provenance_type");
    if (random_json.get_filepath() == "") {
            writer.String("seed");
            writer.Key("generation_seed");
            writer.Int(random_json.get_generation_seed());
    }
    else {
        writer.String("file");
        writer.Key("filename");
        writer.String(random_json.get_filepath().c_str());
    }
    writer.Key("size");
    writer.Int(random_json.get_size());
    writer.Key("mutation_seed");
    writer.Int(random_json.get_mutation_seed());
    writer.Key("number_of_mutations");
    writer.Int(random_json.get_number_of_mutations());
    writer.EndObject();
    writer.Key("parsing_results");
    writer.StartArray();
    for (auto& traverser : traversers) {
        writer.StartObject();
        writer.Key("parser_name");
        writer.String(traverser->get_parser_name().c_str());
        writer.Key("parsing_state");
        writer.String(traverser->get_parsing_state_as_string().c_str());
        if (value_index >= 0) {
            writer.Key("value_index");
            writer.Int(value_index);
            writer.Key("value_type");
            ValueType value_type = traverser->get_current_type();
            writer.String(valuetype_to_string(value_type).c_str());
            switch (value_type) {
                case ValueType::string :
                    writer.String("value");
                    writer.String(traverser->get_string().c_str());
                    break;
                case ValueType::integer :
                    writer.String("value");
                    writer.Int(traverser->get_integer());
                    break;
                case ValueType::floating :
                    writer.String("value");
                    writer.Double(traverser->get_floating());
                    break;
                case ValueType::boolean :
                    writer.String("value");
                    writer.Bool(traverser->get_boolean());
                    break;
            }
        }
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();
    
    // write file
    int size = string_buffer.GetSize();
    std::fstream file(id + time + "_fuzzyreport.json", std::ios::out | std::ios::binary);
    file.write(string_buffer.GetString(), sizeof(char)*size);
    file.close();
    random_json.save(id + time + "_reportedjson.json");
}

}

#endif
