#ifndef SIMDJSONPARSER_H
#define SIMDJSONPARSER_H

#include <string>
#include <unordered_map>

#include "parser.h"
#include "simdjson.h"
#include "simdjson.cpp"

namespace fuzzyjson {

class SimdjsonTraverser : public Traverser {
    public:
    SimdjsonTraverser(std::string parser_name, ParsingResult result, std::unique_ptr<ParsedJson> parsed_json)
    : Traverser(parser_name, result)
    , parsed_json(std::move(parsed_json))
    , iterator(*(this->parsed_json.get()))
    {}

    ~SimdjsonTraverser() override {}

    ValueType get_current_type() {
        switch (iterator.get_type()) {
            case 'r':
                return ValueType::end_of_document;
            case '{':
                return ValueType::object;
            case '[':
                return ValueType::array;
            case '"':
                return ValueType::string;
            case 'l':
                return ValueType::integer;
            case 'd':
                return ValueType::floating;
            case 't':
            case 'f':
                return ValueType::boolean;
            case 'n':
                return ValueType::null;
            case '}':
            case ']':
                return ValueType::end_of_container;
            default :
                return ValueType::error;
        }
    }

    std::string get_string() override {
        return iterator.get_string();
    }
    int get_integer() override {
        return iterator.get_integer();
    }

    double get_floating() override {
        return iterator.get_double();
    }

    bool get_boolean() override {
        return iterator.is_true();
    }

    private:
    void up() override {
        iterator.up();
    }

    void down() override {
        iterator.down();
    }

    void advance_container() override {
        iterator.next();
    }


    ParsedJson::iterator iterator;
    std::unique_ptr<ParsedJson> parsed_json;
};


class SimdjsonParser : public Parser {
    public:
    SimdjsonParser()
    : Parser("simdjson")
    {}

    ~SimdjsonParser() = default;
    
    std::unique_ptr<Traverser> parse(char* const json, int size) override
    {
        ParsedJson* parsed_json = new ParsedJson();
        bool allocation_is_successful = parsed_json->allocateCapacity(size);
        if (!allocation_is_successful) {
            delete parsed_json;
            return std::make_unique<SimdjsonTraverser>(get_name(), ParsingResult::other_error, nullptr);
        }
        auto simdjson_result = static_cast<simdjson::errorValues>(json_parse(json, size, *parsed_json));
        ParsingResult result = result_correspondances.at(simdjson_result);
        return std::make_unique<SimdjsonTraverser>(get_name(), result, std::unique_ptr<ParsedJson>(parsed_json));
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