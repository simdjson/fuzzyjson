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

struct Settings {
    int id = 0; // An identifier for the processus. Used in parallelism.
    int max_mutations = 10000;
    double floating_precision = std::numeric_limits<double>::epsilon();
    bool verbose = false;
};

class FuzzyJson
{
    public:
    FuzzyJson(const randomjson::Settings& json_settings, Settings fuzzy_settings = Settings{});

    void add_parser(std::unique_ptr<Parser> parser) {
        parsers.push_back(std::move(parser));
    }
    void fuzz();

    private:
    void compare_parsing();
    void generate_report(std::vector<std::unique_ptr<Traverser>>& traversers, int value_index);
    bool is_known_problem(std::vector<std::unique_ptr<Traverser>>& traversers);
    void reverse_mutation();
    std::string id;
    randomjson::RandomJson random_json;
    Settings settings;
    std::vector<std::unique_ptr<Parser>> parsers;
};

FuzzyJson::FuzzyJson(const randomjson::Settings& json_settings, Settings fuzzy_settings)
: id(std::to_string(fuzzy_settings.id))
, random_json(json_settings)
, settings(fuzzy_settings)
{}

void FuzzyJson::fuzz()
{
    if (settings.verbose) {
        std::cout << "seed " << random_json.get_generation_seed() << std::endl;
    }
    std::string copy_name = std::string("temp-")+id+".json";
    // mutations and comparisons
    for (int no_mutation = 0; no_mutation != settings.max_mutations; no_mutation++) {
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
    std::vector<std::unique_ptr<Traverser>> traversers;
    for (auto& parser : parsers) {
        traversers.push_back(parser->parse(random_json.get_json(), random_json.get_size()));
    }

    // The first traverser will be compared to all the others.
    ParsingState first_parsing_state = traversers.at(0)->get_parsing_state();

    bool states_are_same = true;
    for (int i = 1; i < traversers.size(); i++) {
        if (traversers.at(0)->get_parsing_state() != traversers.at(i)->get_parsing_state()) {
            states_are_same = false;
        }
    }

    bool to_reverse = false;
    bool report_is_generated = false;

    // To have different states is a problem.
    if (!states_are_same) {
        generate_report(traversers, -1);
        to_reverse = true;
        report_is_generated = true;
    }

    // If all states are same and first parsing failed, then all parsings failed.
    // To have parsers agreeing is not a problem.
    if (states_are_same && first_parsing_state == ParsingState::error) {
        to_reverse = true;
    }

    if (to_reverse) {
        // We can't reverse if there was no mutation
        // There is a problem somewhere. Probably with the json generator.
        if (random_json.get_number_of_mutations() == 0) {
            // We don't generate the same report twice
            if (!report_is_generated) {
                generate_report(traversers, -1);
            }
            // To continue the fuzzing could generate thousands of reports.
            std::abort();
        }

        reverse_mutation();
        return; // we're not interested to compared values
    }

    // Compare values
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
                case ValueType::unsigned_integer : {
                    if (traversers.at(i)->can_handle_unsigned_integer()) {
                        uint64_t leader = traversers.at(0)->get_unsigned_integer();
                        uint64_t other = traversers.at(i)->get_unsigned_integer();
                        different = (leader != other);
                    }
                    break;
                }
                case ValueType::floating : {
                    double leader = traversers.at(0)->get_floating();
                    double other = traversers.at(i)->get_floating();
                    different = !floatings_are_equal(leader, other, settings.floating_precision);
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
    if (is_known_problem(traversers)) {
        // we don't generate a report for a problem we already know about.
        return;
    }

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

bool FuzzyJson::is_known_problem(std::vector<std::unique_ptr<Traverser>>& traversers) {
    for (auto& traverser : traversers) {
        if (traverser->is_known_problem(random_json.get_json(), random_json.get_size())) {
            return true;
        }
    }
    return false;
}

}

#endif
